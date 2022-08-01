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


////////////////////////////////////////////////////////////////////////////////////////
// Frame ///////////////////////////////////////////////////////////////////////////////

#define ftype unsigned char

typedef struct
{
        ftype*   data;
        unsigned width;
        unsigned height;             

} frame;

void frame_pset(frame* o, unsigned x, unsigned y, ftype value)
{
    // if(((x >= 0) && (x < o->width)) && ((y >= 0) && (y < o->height)))
    o->data[x + y * o->width] = value;
}

ftype frame_get(frame* o, unsigned x, unsigned y)
{
    // if(((x >= 0) && (x < o->width)) && ((y >= 0) && (y < o->height))) return o->data[x + y * o->width];
    return o->data[0];
}

void frame_clr(frame* o, ftype value)
{
    for(unsigned i = 0; i < (o->height * o->width); ++i) o->data[i] = value;
}

void frame_init(frame* o, unsigned x, unsigned y)
{
    o->data = (ftype*)calloc(o->width*o->height , sizeof(ftype));
}

void frame_flush(frame* o)
{
    free(o->data);
}
