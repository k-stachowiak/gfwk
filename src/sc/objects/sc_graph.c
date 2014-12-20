/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <stdbool.h>
#include <float.h>
#include <stdlib.h>

#include "diagnostics.h"
#include "memory.h"
#include "random.h"
#include "array.h"

#include "sc_data.h"

#include "sc_graph.h"

static bool op_leq(double x, double y)
{
	return x <= y;
}

static bool op_geq(double x, double y)
{
	return x >= y;
}

static int lgph_find_extreme(
		struct Graph *lgph, struct WorldPos wp,
		double init, bool(*op)(double, double))
{
	double extr_dist = init;
	int index, extr_index = 0;

	for (index = 0; index < lgph->nodes_count; ++index) {

		struct TilePos tile_tp = lgph->nodes[index];
		struct WorldPos tile_wp = pos_tile_to_world(tile_tp);

		double	dx = tile_wp.x - wp.x,
				dy = tile_wp.y - wp.y,
				d = dx * dx + dy * dy;

		if (op(d, extr_dist)) {
			extr_dist = d;
			extr_index = index;
		}
	}

	return extr_index;
}

void lgph_init(
        struct Graph *lgph,
        int nodes_count,
        struct TilePos *nodes,
        struct Adj **adjacency)
{
    lgph->nodes_count = nodes_count;
    lgph->nodes = nodes;
    lgph->adjacency = adjacency;
}

void lgph_deinit(struct Graph *lgph)
{
    int i;
    free_or_die(lgph->nodes);
    for (i = 0; i < lgph->nodes_count; ++i) {
        free_or_die(lgph->adjacency[i]);
    }
    free_or_die(lgph->adjacency);
}

int lgph_find_index(struct Graph *lgph, struct TilePos tp)
{
    int index;
    for (index = 0; index < lgph->nodes_count; ++index) {
        if (lgph->nodes[index].x == tp.x && lgph->nodes[index].y == tp.y) {
            return index;
        }
    }

    DIAG_ERROR("Requested index of node not in graph.");
    exit(1);
}

int lgph_find_random(struct Graph *lgph)
{
	return rnd_uniform_rng_i(0, lgph->nodes_count);
}

int lgph_find_random_skip(struct Graph *lgph, int skipped)
{
	int result;
	do {
		result = lgph_find_random(lgph);
	} while (result == skipped);
	return result;
}

int lgph_find_nearest(struct Graph *lgph, struct WorldPos wp)
{
	return lgph_find_extreme(lgph, wp, DBL_MAX, op_leq);
}

int lgph_find_farthest(struct Graph *lgph, struct WorldPos wp)
{
	return lgph_find_extreme(lgph, wp, 0.0, op_geq);
}

void lgph_dijkstra(
		struct Graph *lgph, struct TilePos src_pos, struct TilePos dst_pos,
		struct TilePos **points, int *points_count)
{
	struct { struct TilePos *data; int cap, size; } result = { NULL, 0, 0 };
	int dst, src, u, i, *preds;
	double *lens;
	bool *visit;

	preds = malloc_or_die(lgph->nodes_count * sizeof(*preds));
	lens = malloc_or_die(lgph->nodes_count * sizeof(*lens));
	visit = malloc_or_die(lgph->nodes_count * sizeof(*visit));

	for (i = 0; i < lgph->nodes_count; ++i) {
		preds[i] = i;
		lens[i] = DBL_MAX;
		visit[i] = false;
	}

	src = u = lgph_find_index(lgph, src_pos);
	lens[src] = 0;

	dst = lgph_find_index(lgph, dst_pos);

	while (u != dst) {
		struct Adj *adj;
		struct TilePos u_pos = lgph->nodes[u];
		double min_len = DBL_MAX;

		visit[u] = true;

		for (adj = lgph->adjacency[u]; adj->neighbor != -1; ++adj) {
			int v = adj->neighbor;
			struct TilePos v_pos = lgph->nodes[v];
			double dx = v_pos.x - u_pos.x, dy = v_pos.y - u_pos.y;
			double distance = sqrt(dx * dx + dy * dy);

			if (lens[u] + distance < lens[v]) {
				lens[v] = lens[u] + distance;
				preds[v] = u;
			}
		}

		for (i = 0; i < lgph->nodes_count; ++i) {
			if (!visit[i] && (lens[i] < min_len)) {
				min_len = lens[i];
				u = i;
			}
		}
	}

	while (u != src) {
		ARRAY_APPEND(result, lgph->nodes[u]);
		u = preds[u];
	}
	ARRAY_APPEND(result, lgph->nodes[u]);

	free_or_die(preds);
	free_or_die(lens);
	free_or_die(visit);

	for (i = 0; i < result.size / 2; ++i) {
		struct TilePos temp = result.data[i];
		result.data[i] = result.data[result.size - i - 1];
		result.data[result.size - i - 1] = temp;
	}

	*points = result.data;
	*points_count = result.size;
}
