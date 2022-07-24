////////////////////////////////////////////////////////////////////////////////////////
// Containers
// V.0.1.2 2022-06-20 (C) Unmanned
////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#ifndef WAVERING_LENGTH
#define WAVERING_LENGTH 128
#endif

////////////////////////////////////////////////////////////////////////////////////////
// Wavering ////////////////////////////////////////////////////////////////////////////
typedef struct
{
    long data[WAVERING_LENGTH];
    unsigned  i; // Write Pointer
    unsigned  o; // Read Pointer

} wavering;

void wavering_init(wavering* o)
{
    o->i = 0;
    o->o = 0;
}

void wavering_set(wavering* o, int value)
{
    o->i++;
    if (o->i >= WAVERING_LENGTH) o->i = 0;
    o->data[o->i] = value;
}

long wavering_get(wavering* o)
{
    o->o++;
    if (o->o >= WAVERING_LENGTH) o->o = 0;
    return o->data[o->o];
}
