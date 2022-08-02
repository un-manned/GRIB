#pragma once
#include "oscillator.h"
#include "sequencer.h"
#include "utility.h"
#include "delay.h"


#define oscn 1
// cvs: 0 = amp
typedef struct
{
    oscillator osc;
    ltfskf     lpf;
    bool  modmatrix[8][6];
    int   waveform[oscn];
    float cvs[8];

    float feed;


} spawner;

void spawner_init(spawner* o)
{
    oscillator_init(&o->osc);
}

void spawn(spawner* o)
{
    form[o->waveform[0]](&o->osc);
    o->feed = o->osc.out * o->cvs[0];
    o->feed = ltfskf_process(&o->lpf, o->feed);
}