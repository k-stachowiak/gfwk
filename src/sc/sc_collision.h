/* Copyright (C) 2014 Krzysztof Stachowiak */

#ifndef SC_COLLISION_H
#define SC_COLLISION_H

#include "sc_level.h"
#include "sc_hunter.h"

struct CollisionContext {
    struct AABB bbox;
    struct VLine lsline, rsline;
    int utiles[3];
    int btiles[3];
    int ltiles[3];
    int rtiles[3];
    struct AABB utile_aabbs[3];
    struct AABB btile_aabbs[3];
    struct AABB ltile_aabbs[3];
    struct AABB rtile_aabbs[3];
};

extern struct CollisionContext cc_last;

void col_handle_all(struct Hunter *hunter, struct Level *lvl);

#endif
