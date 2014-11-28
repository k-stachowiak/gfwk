#include <stdlib.h>
#include <math.h>
#include <float.h>

#include "diagnostics.h"
#include "random.h"
#include "array.h"
#include "sc_graph.h"

static void lgph_dijkstra(
        struct Graph *lgph, struct TilePos src_pos, struct TilePos dst_pos,
        struct TilePos **points, int *points_count)
{
    struct { struct TilePos *data; int cap, size; } result = { NULL, 0, 0 };
    int dst, src, u, i, *preds;
    double *lens;
    bool *visit;

    if (!(preds = malloc(lgph->nodes_count * sizeof(*preds))) ||
        !(lens = malloc(lgph->nodes_count * sizeof(*lens))) ||
        !(visit = malloc(lgph->nodes_count * sizeof(*visit)))) {
            DIAG_ERROR("Allocation failure.");
            exit(1);
    }

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

    free(preds);
    free(lens);
    free(visit);

    for (i = 0; i < result.size / 2; ++i) {
        struct TilePos temp = result.data[i];
        result.data[i] = result.data[result.size - i - 1];
        result.data[result.size - i - 1] = temp;
    }

    *points = result.data;
    *points_count = result.size;
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
    free(lgph->nodes);
    for (i = 0; i < lgph->nodes_count; ++i) {
        free(lgph->adjacency[i]);
    }
    free(lgph->adjacency);
}

int lgph_find_index(struct Graph *lgph, struct TilePos pos)
{
    int index;
    for (index = 0; index < lgph->nodes_count; ++index) {
        if (lgph->nodes[index].x == pos.x && lgph->nodes[index].y == pos.y) {
            return index;
        }
    }

    DIAG_ERROR("Requested index of node not in graph.");
    exit(1);
}

int lgph_find_farthest(struct Graph *lgph, struct TilePos bad)
{
    double max_dist = 0.0;
    int index, max;

    for (index = 0; index < lgph->nodes_count; ++index) {
        struct TilePos pos = lgph->nodes[index];
        double dx = pos.x - bad.x, dy = pos.y - bad.y;
        double distance = sqrt(dx * dx + dy * dy);
        if (distance > max_dist) {
            max_dist = distance;
            max = index;
        }
    }

    return max;
}

void lgph_runaway_path(
        struct Graph *lgph, struct TilePos src, struct TilePos bad,
        struct TilePos **points, int *points_count)
{
    int dst = lgph_find_farthest(lgph, bad);
    lgph_dijkstra(lgph, src, lgph->nodes[dst], points, points_count);
}

void lgph_random_path(
        struct Graph *lgph, struct TilePos src,
        struct TilePos **points, int *points_count)
{
    int dst = rnd_uniform_rng_i(0, lgph->nodes_count - 1);
    lgph_dijkstra(lgph, src, lgph->nodes[dst], points, points_count);
}
