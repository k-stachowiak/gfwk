/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <allegro5/allegro_primitives.h>

#include "cmp_ori.h"
#include "cmp_drv.h"
#include "sc_collision.h"
#include "sc_data.h"

static void vline_to_screen(
    struct VLine vline,
    double *x, double *y1, double *y2)
{
    struct WorldPos wp1 = { vline.x, vline.y1 };
    struct WorldPos wp2 = { vline.x, vline.y2 };
    struct ScreenPos sp1 = pos_world_to_screen(wp1);
    struct ScreenPos sp2 = pos_world_to_screen(wp2);
    *x = sp1.x;
    *y1 = sp1.y;
    *y2 = sp2.y;
}

static void aabb_to_screen(
    struct AABB aabb,
    double *x1, double *y1,
    double *x2, double *y2)
{
    struct WorldPos aaw = { aabb.ax, aabb.ay };
    struct WorldPos bbw = { aabb.bx, aabb.by };
    struct ScreenPos aas = pos_world_to_screen(aaw);
    struct ScreenPos bbs = pos_world_to_screen(bbw);
    *x1 = aas.x;
    *y1 = aas.y;
    *x2 = bbs.x;
    *y2 = bbs.y;
}

static void segment_to_screen(
    struct Segment segment,
    double *x1, double *y1,
    double *x2, double *y2)
{
    struct WorldPos aaw = { segment.ax, segment.ay };
    struct WorldPos bbw = { segment.bx, segment.by };
    struct ScreenPos aas = pos_world_to_screen(aaw);
    struct ScreenPos bbs = pos_world_to_screen(bbw);
    *x1 = aas.x;
    *y1 = aas.y;
    *x2 = bbs.x;
    *y2 = bbs.y;
}

bool aabb_point(struct AABB aabb, double x, double y)
{
    return x < aabb.bx && x > aabb.ax && y < aabb.by && y > aabb.ay;
}

bool aabb_vline(struct AABB aabb, struct VLine vline)
{
    return
        vline.x < aabb.bx && vline.x > aabb.ax &&
        vline.y1 < aabb.by && vline.y2 > aabb.ay;
}

bool aabb_aabb(struct AABB lhs, struct AABB rhs)
{
    return
        lhs.ax < rhs.bx && lhs.bx > rhs.ax &&
        lhs.ay < rhs.by && lhs.by > rhs.ay;
}

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

