/////////////////////////////////////////////////////////////////////////////////////////
// Utilities
// V.0.3.8 2022-07-22
// MIT License
// Copyright (c) 2022 unmanned
/////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#ifndef SAMPLE_RATE 
#define SAMPLE_RATE 44100
#endif
#define PI  3.141592653589793238462f
#define TAO 6.283185307179586476925f

/////////////////////////////////////////////////////////////////////////////////////////
// DC Block filter //////////////////////////////////////////////////////////////////////

float dcb(float in)
{
    static float eax, ebx;
    ebx = in - eax + 0.995 * ebx;
    eax = in;
    return ebx;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Dynamic smoothing self modulating filter /////////////////////////////////////////////
// https://cytomic.com/files/dsp/DynamicSmoothing.pdf ///////////////////////////////////

typedef struct 
{
    float frequency;
    float sensivity;
    float w;
    float u;
    float v;

    float eax;
    float ebx;
    float ecx;
    float edx;

} dssmf;

void init_dssmf(dssmf* o)
{
    o->frequency = 2.0f;
    o->sensivity = 2.0f;
    o->w = o->frequency / SAMPLE_RATE;
    o->u = tanf(PI*o->w);
    o->v = 2.0f * o->u / (1.0f + o->u);
}

float minimum(float a, float b) 
{
    return b > a ? a : b;
}

float process_dssmf(dssmf* o, float in)
{
    o->ecx = o->eax;
    o->edx = o->ebx;
    float b = o->ecx - o->edx;
    float g = minimum(o->v + o->sensivity * fabsf(b), 1.0f);
    o->eax = o->ecx + g*(in - o->ecx);
    o->ebx = o->edx + g*(o->eax - o->edx);
    return o->ebx;
}

/////////////////////////////////////////////////////////////////////////////////////////
// https://cytomic.com/files/dsp/SvfLinearTrapOptimised2.pdf ////////////////////////////
typedef struct 
{
    float ic1eq;
    float ic2eq;
    float g;
    float k;
    float a;
    float b;
    float low;
    float band;
    float high;
    float notch;
    float peak;
    float all;

} ltosvf;

void svflto_clr(ltosvf* o)
{
    o->ic1eq = 0.0f;
    o->ic2eq = 0.0f;
}

void svflto_init(ltosvf* o, float cutoff, float Q)
{
    o->g = tanf(PI * cutoff / SAMPLE_RATE);
    o->k = 1.0f/Q;
    o->a = 1.0f/(1.0f + o->g*(o->g + o->k));
    o->b = o->g * o->a;
}

float svflto_process(ltosvf* o, float in)
{
    float va = o->a*o->ic1eq + o->b*(in - o->ic2eq);
    float vb = o->ic2eq + o->g*va;
    o->ic1eq = 2.0f*va - o->ic1eq;
    o->ic2eq = 2.0f*vb - o->ic2eq;
    o->low   = vb;
    o->band  = va;
    o->high  = in - o->k*va - vb;
    o->notch = o->low + o->high;
    o->peak  = o->low - o->high;
    o->all   = o->low + o->high - o->k*o->band;
}


/////////////////////////////////////////////////////////////////////////////////////////
// https://cytomic.com/files/dsp/SkfLinearTrapOptimised2.pdf ////////////////////////////
typedef struct 
{
    float ic1eq;
    float ic2eq;
    float k;
    float a0, a1, a2, a3, a4, a5;

} ltoskf;

void ltoskf_clr(ltoskf* o)
{
    o->ic1eq = 0.0f;
    o->ic2eq = 0.0f;
}


void ltoskf_init(ltoskf* o, float cutoff, float Q)
{
    float g = tanf(PI * cutoff / SAMPLE_RATE);
    o->k  = Q;
    o->a0 = 1.0f/((1.0f + g)*(1.0f + g)-(g * o->k));
    o->a1 = o->k * o->a0;
    o->a2 = (1.0 + g) * o->a0;
    o->a3 = g * o->a2;
    o->a4 = 1.0f/(1.0f + g);
    o->a5 = g * o->a4;

}

float ltoskf_process(ltoskf* o, float in)
{
    float v1 = o->a1 * o->ic2eq + o->a2*o->ic1eq + o->a3*in;
    float v2 = o->a4 * o->ic2eq + o->a5 * v1;
    o->ic1eq = 2.0f*(v1 - o->k*v2) - o->ic1eq;
    o->ic2eq = 2.0f*(v2          ) - o->ic2eq;
    return v2;
}

/////////////////////////////////////////////////////////////////////////////////////////
// https://cytomic.com/files/dsp/SkfLinearTrapOptimised2.pdf ////////////////////////////

typedef struct 
{
    float ic1eq;
    float ic2eq;
    float g0, g1, g2;

} ltfskf;

void ltfskf_clr(ltfskf* o)
{
    o->ic1eq = 0.0f;
    o->ic2eq = 0.0f;
}

void ltfskf_init(ltfskf* o, float cutoff, float Q)
{
    float w  = PI * cutoff / SAMPLE_RATE;
    float s1 = sinf(w);
    float s2 = sinf(2.0f * w);
    float nrm = 1.0f / (2.f + Q * s2);
    o->g0 = s2 * nrm;
    o->g1 = (-2.f * s1 * s1 - Q * s2) * nrm;
    o->g2 = (2.0f * s1 * s1) * nrm;
}

float ltfskf_process(ltfskf* o, float in)
{
    float t0 = in - o->ic2eq;
    float t1 = o->g0 * t0 + o->g1 * o->ic1eq;
    float t2 = o->g2 * t0 + o->g0 * o->ic1eq;
    float v2 = t2 + o->ic2eq;
    o->ic1eq += 2.0f * t1;
    o->ic2eq += 2.0f * t2;
    return v2;
}

/////////////////////////////////////////////////////////////////////////////////////////
// One pole LP parameter smooth filter //////////////////////////////////////////////////
typedef struct  
{
    float a;
    float b;
    float o;

} psf;

void psf_init(psf* o, float time, float sample_rate)
{
    o->a = expf(-TAO / (time * 0.001f * sample_rate));
    o->b = 1.0f - o->a;
    o->o = 0.0f;
}

float psf_process(psf* o, float in)
{
    o->o = (in * o->b) + (o->o * o->a);
    return o->o;
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
float allpass(float in, float a)
{
    static float y;
    float out = y + a * in;
    y = in - a * out;
    return out;
}    

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
    int t;
    float value;

} snh;

void snh_init(snh* o)
{
    o->t = 0;
    o->value = 0.0f;
}

float snh_process(snh* o, float input, int time)
{
    if (o->t>time)
    {
        o->t = 0;
        o->value = input;
    }
    o->t++;
    return o->value;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Crossfader: f == 1? a = max; f==0? b = max ///////////////////////////////////////////
float crossfade(float a, float b, float f)
{
    return a * f + b * ( 1.0f - f);
}


/////////////////////////////////////////////////////////////////////////////////////////
float saturate(float in, float gain, float drive, float mix)
{
    return crossfade(sinf(tanhf(in * (gain+0.02f)*20.0f) * (drive*1.5f+1.0f)), in, mix);
}


/////////////////////////////////////////////////////////////////////////////////////////
// Departed samples to milliseconds /////////////////////////////////////////////////////
inline float DStoMS(float samples, float sample_rate)
{
    return samples*1000.0f/sample_rate;
}


/////////////////////////////////////////////////////////////////////////////////////////
// Envelope follower ////////////////////////////////////////////////////////////////////
typedef struct
{
    float a;
    float r;
    float envelope;

} ef;

void ef_init(ef* o, float aMs, float rMs)
{
    o->envelope = 0.0f;
    o->a = pow( 0.01, 1.0 / ( aMs * SAMPLE_RATE * 0.001 ) );
    o->r = pow( 0.01, 1.0 / ( rMs * SAMPLE_RATE * 0.001 ) );
}

void ef_process(ef* o, float in)
{
    float f = fabs(in);
    if (f > o->envelope) o->envelope = o->a * ( o->envelope - f ) + f;
    else                 o->envelope = o->r * ( o->envelope - f ) + f;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Limiter //////////////////////////////////////////////////////////////////////////////
typedef struct 
{
    ef e;
    float threshold;

} limiter;


void limiter_init(limiter* o, float aMs, float rMs, float threshold)
{
    ef_init(&o->e, aMs, rMs);
    o->threshold = threshold;
}


float limit(limiter* o, float in)
{
    float out = in;
    ef_process(&o->e, in);
    if(o->e.envelope > o->threshold)
    {
        out /= expf(o->e.envelope - o->threshold);
    }
    return out;
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////