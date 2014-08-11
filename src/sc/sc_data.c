/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <stdio.h>
#include <string.h>

#include "array.h"
#include "diagnostics.h"
#include "sc_data.h"

void *sc_tile;
void *sc_soulbooth;
void *sc_hunter_stand_right;
void *sc_hunter_stand_left;
void *sc_hunter_walk_right;
void *sc_hunter_walk_left;
void *sc_bow_bitmap;
void *sc_arrow_bitmap;
void *sc_soul_stand_right;
void *sc_soul_stand_left;
void *sc_soul_walk_right;
void *sc_soul_walk_left;

int sc_screen_w;
int sc_screen_h;
int sc_tile_w;

struct WorldPos sc_cam_shift;
struct AABB sc_screen_aabb;

struct WorldPos pos_tile_to_world(struct TilePos tile_pos)
{
    struct WorldPos result = {
        tile_pos.x * sc_tile_w,
        tile_pos.y * sc_tile_w
    };
    return result;
}

struct ScreenPos pos_world_to_screen(struct WorldPos world_pos)
{
    struct ScreenPos result = {
        world_pos.x - sc_cam_shift.x + sc_screen_w / 2.0,
        world_pos.y - sc_cam_shift.y + sc_screen_h / 2.0
    };
    return result;
}

void vline_to_screen(
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

void aabb_to_screen(
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

