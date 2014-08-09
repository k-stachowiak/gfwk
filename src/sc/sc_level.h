/* Copyright (C) 2014 Krzysztof Stachowiak */

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

struct LvlGraph {
	int nodes_count;
	struct TilePos *nodes;
	int **adjacency;
};

struct LvlGraph lgph_init(struct Level *lvl);
void lgph_deinit(struct LvlGraph *lgph);
