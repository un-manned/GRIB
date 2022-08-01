////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <math.h>
#include "hardware/pll.h"
#include "hardware/gpio.h"
#include "hardware/clocks.h"
// #include "hardware/structs/clocks.h"
#include "pico/stdlib.h"
#include "pico/audio_i2s.h"
#include "pico/multicore.h"
#include "4051.h"
#include "hardware/adc.h"
// #include "cell/chaos.h"
#include "cell/utility.h"
#include "cell/delay.h"
#include "cell/containers.h"
#include "cell/oscillator.h"
#include "pico-ss-oled/include/ss_oled.h"
////////////////////////////////////////////////////////////////////////////////////
// Globals /////////////////////////////////////////////////////////////////////////
#define WAVE_TABLE_LENGTH   2048
#define SAMPLES_PER_BUFFER  1156
#define SAMPLE_RATE         44100
#define LAG4051             10
////////////////////////////////////////////////////////////////////////////////////
#define BUTTON_C 17
#define BUTTON_B 18
#define BUTTON_A 19

#define DIN_PIN 2
#define BCLK_PIN 0
#define SDA_PIN 4
#define SCL_PIN 5
#define RESET_PIN -1
#define PICO_I2C i2c0
#define I2C_SPEED 100 * 1000

#define OLED_WIDTH 128
#define OLED_HEIGHT 64
////////////////////////////////////////////////////////////////////////////////////

SSOLED oled;
static wavering cbuffer;
static const uint32_t PIN_DCDC_PSM_CTRL = 23;
static int16_t wave_table   [WAVE_TABLE_LENGTH];
audio_buffer_pool_t *ap;

// uint32_t step = 0x200000;
uint32_t step = 0x1A000;
uint32_t pos  = 0;
const uint32_t pos_max = 0x10000 * WAVE_TABLE_LENGTH;
uint vol = 0x5F;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static inline uint32_t _millis(void)
{
	return to_ms_since_boot(get_absolute_time());
}

static frame canvas;
////////////////////////////////////////////////////////////////////////////////////
// Core 1 interrupt Handler ////////////////////////////////////////////////////////
void core1_interrupt_handler() 
{
    // Receive Raw Value, Convert and Print Temperature Value
    float k = SAMPLES_PER_BUFFER/OLED_WIDTH;
    frame_init(&canvas, 128, 64);
    // 
    // 
    while (multicore_fifo_rvalid())
    {
        uint16_t raw = multicore_fifo_pop_blocking();   
        // if(raw == 1)   
        // {
        //     oledFill(&oled, 0,1);
        //     for(int i = 0; i < OLED_WIDTH; i++)
        //     {
        //         oledPSET(&oled, i, wavering_get(&cbuffer)/0x80FFFF+OLED_HEIGHT/2, 0xFF);
        //     }   
        // }
        oledWriteString(&oled, 0, 0, 0, "ABCDEFGHIJKLM", 1, 0, 1);
        oledWriteString(&oled, 0, 0, 1, "NOPQRSTUVWXYZ", 1, 0, 1);
        oledWriteString(&oled, 0, 0, 2, "[0123456789].,", 1, 0, 1);
        oledWriteString(&oled, 0, 0, 3, "MESSAGE4", 1, 0, 1);
        oledWriteString(&oled, 0, 0, 4, "MESSAGE5", 1, 0, 1);
        oledWriteString(&oled, 0, 0, 5, "MESSAGE6", 1, 0, 1);
        oledWriteString(&oled, 0, 0, 6, "MESSAGE7", 1, 0, 1);
        oledWriteString(&oled, 0, 0, 7, "MESSAGE8", 1, 0, 1);
        oledWriteString(&oled, 0, 0, 8, "MESSAGE9", 1, 0, 1);


    }
    multicore_fifo_clear_irq(); // Clear interrupt
}

////////////////////////////////////////////////////////////////////////////////////
// Core 1 Main Code ////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
int main() 
{
    
    stdio_init_all();
    multicore_launch_core1(core1_entry);

    wavering_init(&cbuffer);

    gpio_init(BUTTON_A);
    gpio_init(BUTTON_B);
    gpio_init(BUTTON_C);

    gpio_init(20);
    gpio_init(21);
    gpio_init(22);

    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_set_dir(BUTTON_C, GPIO_IN);

    // gpio_pull_up(BUTTON_A);
    // gpio_pull_up(BUTTON_B);
    // gpio_pull_up(BUTTON_C);

    gpio_set_dir(20, GPIO_OUT);
    gpio_set_dir(21, GPIO_OUT);
    gpio_set_dir(22, GPIO_OUT);
        
    int rc = oledInit(&oled, OLED_128x64, 0x3c, 0, 0, 1, SDA_PIN, SCL_PIN, RESET_PIN, 1000000L);

    oledFill(&oled, 0,1);
    oledSetContrast(&oled, 127);

    adc_init();
    adc_gpio_init(26);
    adc_gpio_init(27);
    adc_gpio_init(28);
    ////////////////////////////////////////////////////////////////////////////////////
    // Set PLL_USB 96MHz ///////////////////////////////////////////////////////////////
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
    ////////////////////////////////////////////////////////////////////////////////////
    for (int i = 0; i < WAVE_TABLE_LENGTH; i++) 
    {
        wave_table[i] = 32767 * cosf(i * 2 * (float) (M_PI / WAVE_TABLE_LENGTH));
    }
    ap = init_audio();
    ////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////
    ulong departed = 0;
    #define NPSF 4
    psf PSF[NPSF];
    for(int i = 0; i < NPSF; i++)
    {
        psf_init(&PSF[i], 2, SAMPLE_RATE);
    }

    delay DD;
    delay_init(&DD);

    static ltfskf FF;
    ltfskf_clr(&FF);

    unsigned note = 1;
    static oscillator osc;
    oscillator_init(&osc);

    // snh SNH;
    // snh_init(&SNH);

    static limiter lim;
    limiter_init(&lim, 0.5f, 3.0f, .5f);

    bool state_a;
    bool state_b;
    bool state_c;

    ////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////
    while (true) 
    {
        departed++;
        if(gpio_get(BUTTON_A)) state_a = true; else state_a = false;
        if(gpio_get(BUTTON_B)) state_b = true; else state_b = false;
        if(gpio_get(BUTTON_C)) state_c = true; else state_c = false;



        adc_select_input(2);

        set4051_2(LAG4051);
        int F = adc_read();

        // if(departed >= F/100) 
        // {
        //     if(note == 1) note = C3N1(F);
        //     else note = C3N1(note);

        //     departed = 0;
        // }
        // step = psf_process(&PSF[0], F)*2000 + 0x1A000;
        // step = 0x1A000;
        // step =  note*128 + 0x1A000;
        // osc.fm = note*0.0001f;

        set4051_4(LAG4051);
        sleep_ms(1);
        DD.time     = adc_read()/4096.0f;
        
        set4051_6(LAG4051);
        float T = adc_read();
        DD.amount   = T/4096.0f*0.9f;
        DD.feedback = T/4096.0f*0.9f;

        set4051_0(LAG4051);
        float cutoff = psf_process(&PSF[1], adc_read() + 0.0f);

        set4051_3(LAG4051);
        float Q = (1.1f - psf_process(&PSF[2], adc_read())/4096.0f*1.05f);

        set4051_5(LAG4051);
        float PW = psf_process(&PSF[3], adc_read()/4096.0f);

        set4051_7(LAG4051);
        osc.warp = adc_read()/4096.0f*0.9f;
        
        set4051_1(LAG4051);
        // int snh_amount = adc_read()/2048.0f;
        // float fm = adc_read()/4096.0f;
        set4051_4(LAG4051);


        osc.delta = TAO/WAVE_TABLE_LENGTH;
        osc.eax   = PI;
        osc.amplitude = 0.5f;
        osc.pwm = (PW - 0.5f) * TAO;
        osc.phase = 0.0f;

        ltfskf_init(&FF, cutoff, Q);

        for (int i = 0; i < WAVE_TABLE_LENGTH; i++) 
        {
            form[3](&osc);
            float out = osc.out*0.5f;
            out = ltfskf_process(&FF, out);
            // out = delay_process(&DD, out);
            out = limit(&lim, out);
            out = dcb(out);
            // osc.fm = snh_process(&SNH, out, F)*snh_amount;
            wave_table[i] = 16384 * out;
        }

        uint16_t raw = 1; //adc_read();
        multicore_fifo_push_blocking(raw);
    }
    delay_clr(&DD);
    // frame_flush(&canvas);
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