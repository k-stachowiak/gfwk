/* Copyright (C) 2014 Krzysztof Stachowiak */

#ifndef SC_GRAPH_H
#define SC_GRAPH_H

#include "sc_data.h"

enum AdjType {
    ADJ_WALK,
    ADJ_JUMP,
};

struct Adj {
    enum AdjType type;
    int neighbor;
};

struct Graph {
	int nodes_count;
	struct TilePos *nodes;
	struct Adj **adjacency; /* Terminated with adj.neighbor == -1 */
};

struct NodeArray {
    struct TilePos *data;
    int size, cap;
};

void lgph_init(
        struct Graph *lgph,
        int nodes_count,
        struct TilePos *nodes,
        struct Adj **adjacency);

void lgph_deinit(struct Graph *lgph);
int lgph_find_index(struct Graph *lgph, struct TilePos pos);
int lgph_find_farthest(struct Graph *lgph, struct TilePos bad);

void lgph_runaway_path(
        struct Graph *lgph, struct TilePos src, struct TilePos bad,
        struct TilePos **points, int *points_count);

void lgph_random_path(
        struct Graph *lgph, struct TilePos src,
        struct TilePos **points, int *points_count);

#endif
