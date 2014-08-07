/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <allegro5/allegro_font.h>

#include "draw.h"

void draw_bitmap(
        void *bitmap,
        double x, double y,
        double csx, double csy,
        double theta)
{
    int w = al_get_bitmap_width((ALLEGRO_BITMAP*)bitmap);
    int h = al_get_bitmap_height((ALLEGRO_BITMAP*)bitmap);
    al_draw_rotated_bitmap(
            (ALLEGRO_BITMAP*)bitmap,
            w / 2 + csx, h / 2 + csy,
            x, y,
            theta,
            0);
}

void draw_text(
        void *font,
        double x, double y,
        double r, double g, double b, double a,
        bool center,
        char *text)
{
    al_draw_text(
        (ALLEGRO_FONT*)font,
        al_map_rgba_f(r, g, b, a),
        x, y,
        center ? ALLEGRO_ALIGN_CENTRE : ALLEGRO_ALIGN_LEFT,
        text);
}
