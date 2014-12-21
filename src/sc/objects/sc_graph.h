/* Copyright (C) 2014 Krzysztof Stachowiak */

#ifndef SC_GRAPH_H
#define SC_GRAPH_H

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
int lgph_find_index(struct Graph *lgph, struct TilePos tp);
int lgph_find_random(struct Graph *lgph);
int lgph_find_random_skip(struct Graph *lgph, int skipped);
int lgph_find_nearest(struct Graph *lgph, struct WorldPos wp);
int lgph_find_farthest(struct Graph *lgph, struct WorldPos wp);

void lgph_dijkstra(
	struct Graph *lgph, struct TilePos src_pos, struct TilePos dst_pos,
	struct TilePos **points, int *points_count);

#endif
