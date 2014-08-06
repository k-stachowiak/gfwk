/* Copyright (C) 2014 Krzysztof Stachowiak */

#ifndef STRING_UTIL_H
#define STRING_UTIL_H

#include <math.h>
#include <stdio.h>
#include <string.h>

/* Based on answer from Stack Overflow */
#define MAX_DOUBLE_DIGITS 1079

inline int string_len_long(long value)
{
    int result;

    if (value == 0) {
        return 1;
    }

    result = log10(fabs(value)) + 1;

    if (value < 0) {
        return result + 1;
    } else {
        return result;
    }
}

inline int string_len_double(double value)
{
    char buffer[MAX_DOUBLE_DIGITS + 1] = { 0 };
    sprintf(buffer, "%f", value);
    return strlen(buffer);
}

#endif
