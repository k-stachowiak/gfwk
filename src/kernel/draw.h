/* Copyright (C) 2014 Krzysztof Stachowiak */

#ifndef DRAW_H
#define DRAW_H

#include <stdbool.h>

void draw_bitmap(
        void *bitmap,
        double x, double y,
        double csx, double csy,
        double theta);

void draw_text(
        void *font,
        double x, double y,
        double r, double g, double b, double a,
        bool center,
        char *text);

#endif
