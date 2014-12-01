/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <allegro5/allegro_primitives.h>

#include "cmp_ori.h"
#include "cmp_drv.h"
#include "sc_collision.h"
#include "sc_data.h"

struct AABB col_tile_aabb(int x, int y)
{
    struct TilePos tp = { x, y };
    struct WorldPos wp = pos_tile_to_world(tp);
    struct AABB aabb = { wp.x, wp.y, wp.x + sc_tile_w, wp.y + sc_tile_w };
    return aabb;
}

void col_draw_aabb(struct AABB aabb, bool fill, double r, double g, double b)
{
    double x1, y1, x2, y2;
    aabb_to_screen(aabb, &x1, &y1, &x2, &y2);
    if (fill) {
        al_draw_filled_rectangle(x1, y1, x2, y2, al_map_rgb_f(r, g, b));
    } else {
        al_draw_rectangle(x1, y1, x2, y2, al_map_rgb_f(r, g, b), 1.0);
    }
}

void col_draw_vline(struct VLine vline, double r, double g, double b)
{
    double x, y1, y2;
    vline_to_screen(vline, &x, &y1, &y2);
    al_draw_line(x, y1, x, y2, al_map_rgb_f(r, g, b), 1.0);
}

void col_draw_segment(struct Segment segment, double r, double g, double b)
{
    double x1, x2, y1, y2;
    segment_to_screen(segment, &x1, &x2, &y1, &y2);
    al_draw_line(x1, x2, y1, y2, al_map_rgb_f(r, g, b), 1.0);
}

