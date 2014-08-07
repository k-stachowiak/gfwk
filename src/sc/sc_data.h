/* Copyright (C) 2014 Krzysztof Stachowiak */

#ifndef SC_DATA_H
#define SC_DATA_H

#include <stdbool.h>

struct TilePos { int x, y; };
struct WorldPos { double x, y; };
struct ScreenPos { double x, y; };

struct WorldPos pos_tile_to_world(struct TilePos tile_pos);
struct ScreenPos pos_world_to_screen(struct WorldPos world_pos);

struct VLine {
    double x;
    double y1, y2;
};

struct AABB {
    double ax, ay, bx, by;
};

void aabb_to_screen(
    struct AABB aabb,
    double *x1, double *y1,
    double *x2, double *y2);

bool aabb_point(struct AABB aabb, double x, double y);
bool aabb_aabb(struct AABB lhs, struct AABB rhs);

extern void *sc_rock_tile;
extern void *sc_hunter_stand;

extern int sc_screen_w;
extern int sc_screen_h;
extern int sc_tile_w;

extern struct WorldPos sc_cam_shift;
extern struct AABB sc_screen_aabb;
extern struct AABB sc_last_hunter_aabb;

#endif
