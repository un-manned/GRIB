#pragma once
#define STEPS 16
#include <math.h>

typedef struct
{
    char  gate[STEPS];  // 1 = on; 0 = off; 2 = hold;
    float note[STEPS];
    int   departed;      // current time in samples
    int   length;        // step duration in samples
    // int   overall;       // overall duration in samples
    int   current;       // current step

} sequencer;

void init_sequence(sequencer* o, int l)
{
    o->departed = 0;
    o->current  = 0;
    o->length   = l;
}

void process_sequence(sequencer* o)
{
    o->departed++;
    if(o->departed == o->length) 
    {
        o->current++;
        if(o->current == STEPS) o->current = 0;
        o->departed = 0;
    }
}

int get_gate(sequencer* o)
{
    return o->gate[o->current];
}

int get_note(sequencer* o)
{
    return o->note[o->current];
}


void genRand(sequencer* o)
{
    for(int i = 0; i < STEPS; i++)
    {
        o->gate[i] = rand()&1;
    }
}