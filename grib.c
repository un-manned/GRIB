#include <stdio.h>
#include <math.h>
#include "hardware/pll.h"
#include "hardware/gpio.h"
#include "hardware/clocks.h"
#include "hardware/structs/clocks.h"
#include "pico/stdlib.h"
#include "pico/audio_i2s.h"
#include "pico/multicore.h"
#include "4051.h"
#include "hardware/adc.h"
#include "cell/chaos.h"
#include "cell/utility.h"
#include "cell/delay.h"
#include "cell/containers.h"
#include "cell/oscillator.h"
#include "pico-ss-oled/include/ss_oled.h"

#define WAVE_TABLE_LENGTH   2048
#define SAMPLES_PER_BUFFER  1156
#define SAMPLE_RATE         44100


#define DIN_PIN 2
#define BCLK_PIN 0
#define SDA_PIN 4
#define SCL_PIN 5
#define RESET_PIN -1
#define PICO_I2C i2c0
#define I2C_SPEED 100 * 1000

#define OLED_WIDTH 128
#define OLED_HEIGHT 64


SSOLED oled;
static wavering cbuffer;

static const uint32_t PIN_DCDC_PSM_CTRL = 23;

static int16_t wave_table   [WAVE_TABLE_LENGTH];

audio_buffer_pool_t *ap;

uint32_t step = 0x200000;
uint32_t pos = 0;

const uint32_t pos_max = 0x10000 * WAVE_TABLE_LENGTH;
uint vol = 0x5F;

audio_buffer_pool_t *init_audio() 
{
    static audio_format_t audio_format = 
    {
            .pcm_format = AUDIO_PCM_FORMAT_S32,
            .sample_freq = SAMPLE_RATE,
            .channel_count = 2
    };

    static audio_buffer_format_t producer_format = 
    {
            .format = &audio_format,
            .sample_stride = 8
    };

    audio_buffer_pool_t *producer_pool = audio_new_producer_pool(&producer_format, 3, SAMPLES_PER_BUFFER); // todo correct size
    
    bool __unused ok;
    const audio_format_t *output_format;

    audio_i2s_config_t config = 
    {
            .data_pin = DIN_PIN,
            .clock_pin_base = BCLK_PIN,
            .dma_channel = 0,
            .pio_sm = 0
    };

    output_format = audio_i2s_setup(&audio_format, &audio_format, &config);
    
    if (!output_format) 
    {
        panic("PicoAudio: Unable to open audio device.\n");
    }

    ok = audio_i2s_connect(producer_pool);
    assert(ok);
    { 
        // initial buffer data
        audio_buffer_t *buffer = take_audio_buffer(producer_pool, true);
        int32_t *samples = (int32_t *) buffer->buffer->bytes;
        for (uint i = 0; i < buffer->max_sample_count; i++) 
        {
            samples[i*2+0] = 0;
            samples[i*2+1] = 0;
        }
        buffer->sample_count = buffer->max_sample_count;
        give_audio_buffer(producer_pool, buffer);
    }
    audio_i2s_set_enabled(true);

    return producer_pool;
}

static inline uint32_t _millis(void)
{
	return to_ms_since_boot(get_absolute_time());
}



void bresenham(int x0, int y0, int x1, int y1) 
{
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = (dx > dy ? dx : -dy) / 2;

    while (oledSetPixel(&oled, x0, y0, 0xFF, 1), x0 != x1 || y0 != y1) 
    {
        int e2 = err;
        if (e2 > -dx) { err -= dy; x0 += sx; }
        if (e2 <  dy) { err += dx; y0 += sy; }
    }
}

// Core 1 interrupt Handler
void core1_interrupt_handler() 
{
    // Receive Raw Value, Convert and Print Temperature Value
    float k = SAMPLES_PER_BUFFER/OLED_WIDTH;
    while (multicore_fifo_rvalid())
    {
        // char buffer[10];
        // int ret = snprintf(buffer, sizeof buffer, "%f", F);
        uint16_t raw = multicore_fifo_pop_blocking();   
        if(raw == 1)   
        {
            oledFill(&oled, 0,1);
            for(int i = 0; i < OLED_WIDTH; i++)
            {
                oledSetPixel(&oled, i, wavering_get(&cbuffer)/0x80FFFF+OLED_HEIGHT/2, 0xFF, 1);
            }   
            // oledWriteString(&oled, 0,0,0, buffer, FONT_8x8, 0, 1);
        }
    }
    multicore_fifo_clear_irq(); // Clear interrupt
}

// Core 1 Main Code
void core1_entry() 
{
    // Configure Core 1 Interrupt
    multicore_fifo_clear_irq();
    irq_set_exclusive_handler(SIO_IRQ_PROC1, core1_interrupt_handler);
    irq_set_enabled(SIO_IRQ_PROC1, true);
    // Infinte While Loop to wait for interrupt
    while (true)
    {
        tight_loop_contents();
    }
}



int main() 
{
    stdio_init_all();
    multicore_launch_core1(core1_entry);

    wavering_init(&cbuffer);

    gpio_init(20);
    gpio_init(21);
    gpio_init(22);

    gpio_set_dir(20, GPIO_OUT);
    gpio_set_dir(21, GPIO_OUT);
    gpio_set_dir(22, GPIO_OUT);
        
    int rc = oledInit(&oled, OLED_128x64, 0x3c, 0, 0, 1, SDA_PIN, SCL_PIN, RESET_PIN, 1000000L);

    oledFill(&oled, 0,1);
    oledSetContrast(&oled, 127);
    // oledWriteString(&oled, 0,0,0,(char *)"**************** ", FONT_8x8, 0, 1);

    adc_init();
    // Make sure GPIO is high-impedance, no pullups etc
    adc_gpio_init(26);
    adc_gpio_init(27);
    adc_gpio_init(28);

    // Set PLL_USB 96MHz
    pll_init(pll_usb, 1, 1536 * MHZ, 4, 4);
    clock_configure(clk_usb,
        0,
        CLOCKS_CLK_USB_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB,
        96 * MHZ,
        48 * MHZ);
    // Change clk_sys to be 96MHz.
    clock_configure(clk_sys,
        CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX,
        CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB,
        96 * MHZ,
        96 * MHZ);
    // CLK peri is clocked from clk_sys so need to change clk_peri's freq
    clock_configure(clk_peri,
        0,
        CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLK_SYS,
        96 * MHZ,
        96 * MHZ);
    // Reinit uart now that clk_peri has changed
    stdio_init_all();
    ////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////


    // DCDC PSM control
    // 0: PFM mode (best efficiency)
    // 1: PWM mode (improved ripple)
    gpio_init(PIN_DCDC_PSM_CTRL);
    gpio_set_dir(PIN_DCDC_PSM_CTRL, GPIO_OUT);
    gpio_put(PIN_DCDC_PSM_CTRL, 1); // PWM mode for less Audio noise

    for (int i = 0; i < WAVE_TABLE_LENGTH; i++) 
    {
        wave_table[i] = 32767 * cosf(i * 2 * (float) (M_PI / WAVE_TABLE_LENGTH));
    }

    ap = init_audio();
    ulong departed = 0;
    psf PSF[4];
    psf_init(&PSF[0], 2, SAMPLE_RATE);
    psf_init(&PSF[1], 2, SAMPLE_RATE);
    psf_init(&PSF[2], 2, SAMPLE_RATE);
    psf_init(&PSF[3], 2, SAMPLE_RATE);
    delay DD;
    delay_init(&DD);

    static ltfskf FF;
    ltfskf_clr(&FF);

    unsigned note = 1;
    static oscillator osc;
    oscillator_init(&osc);

    snh SNH;
    snh_init(&SNH);

    static limiter lim;
    limiter_init(&lim, 0.5f, 3.0f, .5f);
    while (true) 
    {
        departed++;
        
        adc_select_input(2);

        set4051(2);
        sleep_ms(1);
        int F = adc_read();

        // if(departed >= F/100) 
        // {
        //     if(note == 1) note = C3N1(F);
        //     else note = C3N1(note);

        //     departed = 0;
        // }
        // step = psf_process(&PSF[0], F)*2000 + 0x1A000;
        step = 0x1A000;
        // step =  note*128 + 0x1A000;
        // osc.fm = note*0.0001f;

        set4051(4);
        sleep_ms(1);
        DD.time     = adc_read()/4096.0f;
        // set4051(7);
        // sleep_ms(1);
        // 
        
        set4051(6);
        sleep_ms(1);
        float T = adc_read();
        //DD.time     = T/262144.0f;
        DD.amount   = T/4096.0f*0.9f;
        DD.feedback = T/4096.0f*0.9f;

        set4051(0);
        sleep_ms(1);
        float cutoff = psf_process(&PSF[1], adc_read() + 0.0f);

        set4051(3);
        sleep_ms(1);
        float Q = (1.1f - psf_process(&PSF[2], adc_read())/4096.0f*1.05f);

        set4051(5);
        sleep_ms(1);
        float PW = psf_process(&PSF[3], adc_read()/4096.0f);

        ltfskf_init(&FF, cutoff, Q);

        set4051(7);
        sleep_ms(1);
        // float snhtime = adc_read()*10.0f;
        osc.warp = adc_read()/4096.0f*0.9f;
        
        
        set4051(1);
        sleep_ms(1);
        int snh_amount = adc_read()/2048.0f;


        // D = adc_read();
        float fm = adc_read()/4096.0f;
        set4051(4);
        sleep_ms(1);
        __helmholz.t = (float)adc_read()/100000.0f + 0.00001f;
        // set4051(2);
        // float amp = adc_read()*4;
        
        // set_delta(&osc, 220);
        //form[1](&osc);
        osc.delta = TAO/WAVE_TABLE_LENGTH;
        osc.eax   = PI;
        osc.amplitude = 0.5f;
        osc.pwm = (PW - 0.5f) * TAO;
        osc.phase = 0.0f;

        // step *= snh_process(&SNH, out, snhtime)
        for (int i = 0; i < WAVE_TABLE_LENGTH; i++) 
        {
            // fHelmholz(&__helmholz);
            // DD.time = __helmholz.y * 0.1f;
            // fSprott(&__sprott);
            // fm *= __sprott.y;
            // wave_table[i] = 32767 * (ltoskf_process(&FF, cosf(i * 2 * (float) (M_PI / WAVE_TABLE_LENGTH))));
            // osc.fm = __helmholz.y * fm;
            
            form[3](&osc);
            float out = osc.out*0.5f;

            out = ltfskf_process(&FF, out);
            out = delay_process(&DD, out);
            
            out = limit(&lim, out);
            out = dcb(out);
            
            
            osc.fm = snh_process(&SNH, out, F)*snh_amount;
            // out = ltfskf_process(&FF, out);
            wave_table[i] = 16384 * out;
        }

        uint16_t raw = 1; //adc_read();
        multicore_fifo_push_blocking(raw);
    }
    delay_clr(&DD);
    return 0;
}

void i2s_callback_func()
{
    audio_buffer_t *buffer = take_audio_buffer(ap, false);
    if (buffer == NULL) { return; }
    int32_t *samples = (int32_t *) buffer->buffer->bytes;
    for (uint i = 0; i < buffer->max_sample_count; i++) 
    {
        samples[i*2+0] = (vol * wave_table[pos >> 16u]) << 8u;;  // L
        samples[i*2+1] = (vol * wave_table[pos >> 16u]) << 8u;;  // R

        if(i%0xF==0) wavering_set(&cbuffer, samples[i*2]);

        pos += step;
        if (pos >= pos_max) pos -= pos_max;
    }
    buffer->sample_count = buffer->max_sample_count;
    give_audio_buffer(ap, buffer);
    return;
}