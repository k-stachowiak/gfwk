/* Copyright (C) 2014 Krzysztof Stachowiak */

#ifndef SC_COLLISION_H
#define SC_COLLISION_H

#include "sc_level.h"
#include "sc_hunter.h"
#include "sc_soul.h"

struct AABB col_tile_aabb(int x, int y);
void col_draw_aabb(struct AABB aabb, bool fill, double r, double g, double b);
void col_draw_vline(struct VLine vline, double r, double g, double b);
void col_draw_segment(struct Segment segment, double r, double g, double b);

#endif
