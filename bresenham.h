#pragma once
#include "pico/stdlib.h"

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