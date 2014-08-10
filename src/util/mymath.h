#ifndef MYMATH_H
#define MYMATH_H

/* The infamous fast inverse square root. */
static inline float rsqrt(float number)
{
    long i;
    float x2, y;
    float threehalfs = 1.5f;
    x2 = number * 0.5f;
    y = number;
    i = *(long*) &y;
    i = 0x5f3759df - (i >> 1);
    y = *(float*)&i;
    y = y * (threehalfs - (x2 * y * y));
    y = y * (threehalfs - (x2 * y * y));
    return y;
}

#endif
