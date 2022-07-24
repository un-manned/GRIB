#pragma once
#include <math.h>
#include "utility.h"
#define DELAY_LENGTH 32768

typedef struct
{
    float* data;
    float  tmax;
    int    sample;   // Sample o->sample
    float  feedback; // 0.0f is endless; 1.0f is one tick
    float  amount;
    float  time;

} delay;

void delay_init(delay* o)
{
    o->tmax = DELAY_LENGTH/1;
    o->data = (float*)calloc(DELAY_LENGTH, sizeof(float));
    o->sample   = 0;
    o->feedback = 0.5f;
    o->amount   = 0.5f;
}

void delay_clr(delay* o) 
{
    free(o->data);
}

float delay_process(delay* o, float input)
{
    if (o->sample >= DELAY_LENGTH) o->sample = 0;
    int f = o->sample - roundf(o->time * o->tmax);
    if (f < 0) f += DELAY_LENGTH;
    float out = o->data[o->sample] = input + (o->data[f] * o->feedback);
    o->sample++;
    return crossfade(out, input, o->amount);
}