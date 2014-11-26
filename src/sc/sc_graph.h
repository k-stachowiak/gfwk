/* Copyright (C) 2014 Krzysztof Stachowiak */

#ifndef SC_GRAPH_H
#define SC_GRAPH_H

#include "sc_data.h"

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
	struct LvlAdj **adjacency; /* Terminated with adj.neighbor == -1 */
};

struct NodeArray {
    struct TilePos *data;
    int size, cap;
};

void lgph_init(
        struct LvlGraph *lgph,
        int nodes_count,
        struct TilePos *nodes,
        struct LvlAdj **adjacency);

void lgph_deinit(struct LvlGraph *lgph);
int lgph_find_index(struct LvlGraph *lgph, struct TilePos pos);
int lgph_find_farthest(struct LvlGraph *lgph, struct TilePos bad);

void lgph_runaway_path(
        struct LvlGraph *lgph, struct TilePos src, struct TilePos bad,
        struct TilePos **points, int *points_count);

void lgph_random_path(
        struct LvlGraph *lgph, struct TilePos src,
        struct TilePos **points, int *points_count);

#endif
