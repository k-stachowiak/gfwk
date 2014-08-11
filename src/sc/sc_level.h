/* Copyright (C) 2014 Krzysztof Stachowiak */

#ifndef SC_LEVEL_H
#define SC_LEVEL_H

#include "sc_data.h"

struct Level {
    int width, height;
    char *map;
    int map_size;
};

void lvl_load(struct Level *lvl, char *filename);
void lvl_unload(struct Level *lvl);
void lvl_for_each_tile(struct Level *lvl, void (*f)(struct TilePos, char));
int lvl_get_tile(struct Level *lvl, int x, int y);
void lvl_draw(struct Level *lvl);

enum LvlAdjType {
    LVL_ADJ_WALK,
    LVL_ADJ_JUMP,
};

struct LvlAdj {
    enum LvlAdjType type;
    int neighbor;
};

struct LvlGraph {
	int nodes_count;
	struct TilePos *nodes;
	struct LvlAdj **adjacency;
};

struct LvlGraph lgph_init(struct Level *lvl);
void lgph_deinit(struct LvlGraph *lgph);
int lgph_find_index(struct LvlGraph *lgph, struct TilePos pos);
int lgph_find_farthest(struct LvlGraph *lgph, struct TilePos bad);

void lgph_runaway_path(
        struct LvlGraph *lgph, struct TilePos src, struct TilePos bad,
        struct TilePos **points, int *points_count);

void lgph_arbitrary_path(
        struct LvlGraph *lgph, struct TilePos src,
        struct TilePos **points, int *points_count);

#endif