////////////////////////////////////////////////////////////////////////////////////////
// Oscillator
// V.0.3.7 2022-06-15
////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include <math.h>

#ifndef WAVE_TABLE_LENGTH
#define WAVE_TABLE_LENGTH 2048
#endif

#ifndef SAMPLE_RATE 
#define SAMPLE_RATE 44100
#endif
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
#define TAO 6.283185307179586476925f
#define PI  3.141592653589793238462f


typedef struct
{
    float* data;          // Wavetable
    int    width;         // Wavetable width
    int    nharm;         // Number of harmonics
    float  phase;         // Current phase
    float  delta;         // Phase increment

    float  frequency;     // Frequency in Hz
    float  amplitude;     // Overall amplitude
    float  eax, ebx, ecx; // Feedback memory
    float  fm;            // Frequency modulation
    float  am;            // Amplitude modulation
    float  pwm;           // Pulse width modulation: 0 < 1 
    float  warp;          // Form dependent modulation: 0 < 1
    float  out;           // Output: x, y, z

} oscillator; 

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
void oscillator_init(oscillator* o)
{ 
    o->nharm = 8;
    o->phase = 0;
    o->amplitude = 1.0f;
    o->fm   = 0.0f;
    o->am   = 0.0f;
    o->pwm  = 0.0f;
    o->warp = 0.0f;
}

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
void set_delta(oscillator* o, const float Hz)
{ 
    o->frequency = Hz;
    o->delta = o->frequency * TAO / SAMPLE_RATE; 
}


/////////////////////////////////////////////////
// 1D ///////////////////////////////////////////
void oSine(oscillator*);
void oRamp(oscillator*);
void oSawtooth(oscillator*);
void oSquare(oscillator*);
void oTomisawa(oscillator*);
void oTriangle(oscillator*);


void (*form[])(oscillator*) = 
{    
    oSine,          // 0
    oRamp,          // 1
    oSawtooth,      // 2
    oSquare,        // 3
    oTomisawa,      // 4
    oTriangle       // 5
};

////////////////////////////////////////////////////////////////////////////////////////
// Waveforms: VCO //////////////////////////////////////////////////////////////////////

void oSine(oscillator* o)
{       
    o->out = sinf(o->phase) * o->amplitude;
    o->phase += o->delta + o->fm;
    if(o->phase >= TAO) o->phase -= TAO;
}


void oSineWT(oscillator* o)
{
    float delta = o->delta;
    float x = o->amplitude * cosf(o->phase);
    float y = o->amplitude * sinf(o->phase);
    float cs = cosf(delta);
    float sn = sinf(delta);

    for(int i = 1; i < o->width; ++i)
    {
        delta = x;
        x = cs*x - sn*y;     // x = samples of a*cos(2*pi*f*t + p)
        y = sn*delta + cs*y; // y = samples of a*sin(2*pi*f*t + p)
        o->data[i] = y;
    }
}

void oParabolWT(oscillator* o)
{
    float amp = o->amplitude;
    int m = SAMPLE_RATE/(2.0 * o->frequency);
    int a = -m;
    int b = 0;

    amp *= 4.0 / (float)(m*m);
    for(int i = 0; i < o->width; i++)
    {
        if( i%m == 0 ) 
        { 
            a+=m; 
            b+=m; 
            amp=-amp; 
        }
        o->data[i] = amp * (i-a) * (i-b);
    }
}


void oRamp(oscillator* o) 
{
    o->out = o->phase/PI * o->amplitude;
    o->phase += o->delta + o->fm;
    if(o->phase >= PI) o->phase -= TAO;
}

void oSawtooth(oscillator* o) 
{
    o->out = - o->phase/PI * o->amplitude;
    o->phase += o->delta + o->fm;
    if(o->phase >= PI) o->phase -= TAO;
}

void oSquare(oscillator* o)
{
    float saw  =  o->phase/PI;
    float ramp =   -o->eax/PI;

    o->out = saw * (o->warp) + ramp * (1.0f - o->warp);

    o->phase += o->delta + o->fm;
    o->eax   += o->delta + o->fm;

    if(o->phase >= (PI)) o->phase -= TAO;
    if(o->eax   >= (PI + o->pwm)) o->eax -= TAO;
}


void oTomisawa(oscillator* o)
{
    o->ecx = 1.0f;                  
    o->ecx *= 1.0f * (1 - 0.0001f * o->frequency); 
    if(o->ecx < 0) o->ecx = 0;

    o->phase += o->delta + o->fm;                
    if(o->phase >= PI) o->phase -= TAO;             

    float oa = cos(o->phase + o->ecx * o->eax); 
    o->eax = 0.5f*(oa + o->eax);        

    float ob = cos(o->phase + o->ecx * o->ebx + (o->pwm * 1.9f + 0.05f) * PI); 
    o->ebx = 0.5f * (ob + o->ebx);            
    o->out = (oa - ob) * o->amplitude;
}

void oTriangle(oscillator* o)
{
    float rise = o->pwm * TAO;
    float fall = TAO - rise;
    float rise_delta = (rise != 0.0f) ? (2.0f * o->amplitude / rise) : 0.0f;
    float fall_delta = (fall != 0.0f) ? (2.0f * o->amplitude / fall) : 0.0f;

    if (o->phase > TAO) o->phase -= TAO;
    if (o->phase < rise) o->out = - o->amplitude + sqrt(o->phase) * rise_delta;
    else o->out = o->amplitude - (sqrt(o->phase) - rise) * fall_delta;
    o->phase += o->delta + o->fm;
}


// void oGinger(oscillator* o)     // Add warp ????
// {
//         static gingerbreadman g;
//         g.x =   o->phase;
//         g.y*= (*o->pwm - 0.5f) * 2.0f;
//         g.iterate();
//         o->out.c = (g.x - 2.5f) * *o->amplitude;

//         o->phase += o->delta + *o->fm;
//         if(o->phase >= PI) o->phase -= TAO;
// }

// void oIkeda(oscillator* o)     // *
// {
//         static ikeda i;
//         i.x =  o->phase;
//         i.u = (0.979999f + *o->pwm/50.0f);
        
//         i.iterate();
//         o->out.c = (i.y + i.x * o->phase) * *o->amplitude;

//         o->phase += o->delta + *o->fm;
//         if(o->phase >= PI) o->phase -= TAO;
// }


// void oDuffing(oscillator* o)     // *
// {
//         static duffing d;
//         d.y =  o->phase;
//         //d.t = o->delta/10;
//         d.a = *o->pwm*16.0f;
//         d.iterate();
//         o->out.c = (d.y*0.1f) * *o->amplitude;

//         o->phase += o->delta + *o->fm;
//         if(o->phase >= PI) o->phase -= TAO;
// }



// void oFabrikant(oscillator* o)     
// {
//         static rabinovich_fabrikant d;
//         static dcb f;
//         d.x = o->phase;
//         d.gamma = (0.979999f +o->phase/50.0f);
//         d.x *= (1.0f + *o->pwm);
//         //d.gamma = 0.999999;
//         //d.alpha = (o->pwmcv-0.5)*8;
//         d.iterate();
//         o->out.c = 5 * (f.process((d.x+d.y+d.z)*0.002f* *o->amplitude*(1.01-*o->pwm)*(1.2-o->phase)));

//         o->phase += o->delta + *o->fm;
//         if(o->phase >= PI) o->phase -= TAO;
// }

// void oVanDerPol(oscillator* o)// OK
// {
//         static vanderpol d;

//         d.t = o->delta / 3.0f;
//         d.f = o->phase * 2.0f;
//         //d.a = 10.0f + o->phasecv*2;
//         d.iterate();
        
//         o->out.c = ( xfade( d.y, d.x, *o->pwm )) * *o->amplitude;
//         o->phase += o->delta + *o->fm;
//         if(o->phase >= PI) o->phase -= TAO;
// }



// ////////////////////////////////////////////////////////////////////////////////////////
// // 3D //////////////////////////////////////////////////////////////////////////////////

// void oSprottST(oscillator* o)
// {
//         static sprott_st s;

//         s.t = o->delta/2.0f;
//         s.iterate();
        
//         o->out.l = s.x * *o->amplitude;
//         o->out.c = s.y * *o->amplitude;
//         o->out.r = s.z * *o->amplitude;

//         o->phase += o->delta + *o->fm;
//         if(o->phase >= PI) o->phase -= TAO;
// }

// void oHelmholz(oscillator* o)
// {
//         static helmholz d;

//         d.t = o->delta/2.0f;
//         d.gamma = ((*o->pwm-0.5f)*2.0f)  + 5.11f;
//         d.delta = ((o->warp-0.5f)*0.03f) + 0.55f;
//         d.iterate();
        
//         o->out.l = d.x * *o->amplitude * 3.0f;
//         o->out.c = d.y * *o->amplitude * 3.0f;
//         o->out.r = d.z * *o->amplitude * 3.0f;

//         o->phase += o->delta + *o->fm;
//         if(o->phase >= PI) o->phase -= TAO;
// }

// void oHalvorsen(oscillator* o)
// {
//         static halvorsen h;

//         h.t = o->delta/20.0f;
//         h.iterate();
        
//         o->out.l = h.x * *o->amplitude * 0.5f;
//         o->out.c = h.y * *o->amplitude * 0.5f;
//         o->out.r = h.z * *o->amplitude * 0.5f;

//         o->phase += o->delta + *o->fm;
//         if(o->phase >= PI) o->phase -= TAO;
// }

// void oTSUCS(oscillator* o)
// {
//         static tsucs d;
//         d.b = *o->pwm   / 2.0f + 0.40f;
//         d.e = o->warp  / 8.0f + 0.55f;
//         d.t = o->delta / 40.0f;
//         d.iterate();

//         o->out.l = (d.x) * *o->amplitude * 0.1f;
//         o->out.c = (d.y) * *o->amplitude * 0.1f;
//         o->out.r = (d.z-28.0f) * *o->amplitude * 0.1f;

//         o->phase += o->delta + *o->fm;
//         if(o->phase >= PI) o->phase -= TAO;

//         // o->latitude += o->theta + o->fm;
//         // if(o->latitude >= TAO) o->latitude -= TAO;
// }


// void oLinz(oscillator* o)
// {
//         static linz l;

//         l.t = o->delta;
//         l.iterate();
        
//         o->out.l = l.x * *o->amplitude;
//         o->out.c = l.y * *o->amplitude;
//         o->out.r = l.z * *o->amplitude;

//         o->phase += o->delta + *o->fm;
//         if(o->phase >= PI) o->phase -= TAO;
// }

// void oYuWang(oscillator* o)
// {
//         static yu_wang yu;

//         yu.t = o->delta/40;
//         yu.iterate();
        
//         o->out.l = yu.x * *o->amplitude;
//         o->out.c = yu.y * *o->amplitude;
//         o->out.r = yu.z * *o->amplitude;

//         o->phase += o->delta + *o->fm;
//         if(o->phase >= PI) o->phase -= TAO;
// }

// void oRoessler(oscillator* o)
// {
//         static roessler rsl;

//         rsl.delta = o->delta;
//         rsl.iterate();
        
//         o->out.l = rsl.x * *o->amplitude * 0.5f;
//         o->out.c = rsl.y * *o->amplitude * 0.5f;
//         o->out.r = rsl.z * *o->amplitude * 0.5f;

//         o->phase += o->delta + *o->fm;
//         if(o->phase >= PI) o->phase -= TAO;
// }

// void oLorenz(oscillator* o)
// {
//         static lorenz lr;

//         lr.t = o->delta;
//         lr.iterate();
        
//         o->out.l = lr.x * *o->amplitude * 0.25f;
//         o->out.c = lr.y * *o->amplitude * 0.25f;
//         o->out.r = lr.z * *o->amplitude * 0.25f;

//         o->phase += o->delta + *o->fm;
//         if(o->phase >= PI) o->phase -= TAO;
// }

// void oChenLee(oscillator* o)
// {
//         static chen_lee cl;

//         cl.t = o->delta;
//         cl.iterate();
        
//         o->out.l = cl.x * *o->amplitude * 0.25f;
//         o->out.c = cl.y * *o->amplitude * 0.25f;
//         o->out.r = cl.z * *o->amplitude * 0.25f;

//         o->phase += o->delta + *o->fm;
//         if(o->phase >= PI) o->phase -= TAO;
// }

