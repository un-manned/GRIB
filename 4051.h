#pragma once
#include "pico/stdlib.h"

#define PIN_A 20
#define PIN_B 21
#define PIN_C 22

void set4051(int n)
{
    if(n == 0)
    {
        gpio_put(PIN_A, 0);
        gpio_put(PIN_B, 0);
        gpio_put(PIN_C, 0);
    }
    else if(n == 1)
    {
        gpio_put(PIN_A, 0);
        gpio_put(PIN_B, 0);
        gpio_put(PIN_C, 1);
    }
    else if(n == 2)
    {
        gpio_put(PIN_A, 0);
        gpio_put(PIN_B, 1);
        gpio_put(PIN_C, 0);
    }
    else if(n == 3)
    {
        gpio_put(PIN_A, 0);
        gpio_put(PIN_B, 1);
        gpio_put(PIN_C, 1);
    }
    else if(n == 4)
    {
        gpio_put(PIN_A, 1);
        gpio_put(PIN_B, 0);
        gpio_put(PIN_C, 0);
    }
    else if(n == 5)
    {
        gpio_put(PIN_A, 1);
        gpio_put(PIN_B, 0);
        gpio_put(PIN_C, 1);
    }
    else if(n == 6)
    {
        gpio_put(PIN_A, 1);
        gpio_put(PIN_B, 1);
        gpio_put(PIN_C, 0);
    }
    else if(n == 7)
    {
        gpio_put(PIN_A, 1);
        gpio_put(PIN_B, 1);
        gpio_put(PIN_C, 1);
    }
}
