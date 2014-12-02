/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <math.h>

#include <allegro5/allegro_primitives.h>

#include "cmp_ori.h"
#include "cmp_drv.h"
#include "sc_collision.h"
#include "sc_data.h"

static inline double dot(double x1, double y1, double x2, double y2)
{
    return x1 * x2 + y1 * y2;
}

static inline void point_to_screen(double xi, double yi, double *xo, double *yo)
{
    struct WorldPos wp = { xi, yi };
    struct ScreenPos sp = pos_world_to_screen(wp);
    *xo = sp.x;
    *yo = sp.y;
}

static void vline_to_screen(
    struct VLine vline,
    double *x, double *y1, double *y2)
{
    point_to_screen(vline.x, vline.y1, x, y1);
    point_to_screen(vline.x, vline.y2, x, y2);
}

static void aabb_to_screen(
    struct AABB aabb,
    double *x1, double *y1,
    double *x2, double *y2)
{
    point_to_screen(aabb.ax, aabb.ay, x1, y1);
    point_to_screen(aabb.bx, aabb.by, x2, y2);
}

static void segment_to_screen(
    struct Segment segment,
    double *x1, double *y1,
    double *x2, double *y2)
{
    point_to_screen(segment.ax, segment.ay, x1, y1);
    point_to_screen(segment.bx, segment.by, x2, y2);
}

bool col_aabb_point(struct AABB aabb, double x, double y)
{
    return x < aabb.bx && x > aabb.ax && y < aabb.by && y > aabb.ay;
}

bool col_aabb_vline(struct AABB aabb, struct VLine vline)
{
    return
        vline.x < aabb.bx && vline.x > aabb.ax &&
        vline.y1 < aabb.by && vline.y2 > aabb.ay;
}

bool col_aabb_aabb(struct AABB lhs, struct AABB rhs)
{
    return
        lhs.ax < rhs.bx && lhs.bx > rhs.ax &&
        lhs.ay < rhs.by && lhs.by > rhs.ay;
}

bool col_segment_circle(struct Segment seg, struct Circle cir)
{
    double dx = seg.bx - seg.ax;
    double dy = seg.by - seg.ay;
    double fx = seg.ax - cir.x;
    double fy = seg.ay - cir.y;

    double a = dot(dx, dy, dx, dy);
    double b = 2 * dot(fx, fy, dx, dy) ;
    double c = dot(fx, fy, fx, fy) - cir.r * cir.r ;

    double delta = b * b - 4 * a * c;

    if (delta < 0) {
        return false;
    }

    double sqr_delta = sqrt(delta);

    double t1 = (-b - sqr_delta) / (2 * a);
    double t2 = (-b + sqr_delta) / (2 * a);

    if (t1 >= 0 && t1 <= 1) {
        return true;
    }

    if (t2 >= 0 && t2 <= 1) {
        return true;
    }

    return false;
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

void col_draw_circle(struct Circle cir, double r, double g, double b)
{
    double x, y;
    point_to_screen(cir.x, cir.y, &x, &y);
    al_draw_circle(x, y, cir.r, al_map_rgb_f(r, g, b), 1.0);
}

