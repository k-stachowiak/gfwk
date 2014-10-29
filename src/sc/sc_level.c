/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <stdio.h>
#include <string.h>
#include <float.h>
#include <math.h>

#include <allegro5/allegro_primitives.h>

#include "random.h"
#include "draw.h"
#include "array.h"
#include "diagnostics.h"
#include "sc_level.h"

static inline double min(double a, double b)
{
    return (a < b) ? a : b;
}

static inline double max(double a, double b)
{
    return (a < b) ? b : a;
}

static inline bool eq_tilepos(struct TilePos *a, struct TilePos *b)
{
	return a->x == b->x && a->y == b->y;
}

static void lvl_draw_tile(struct TilePos tile_pos, char c)
{
    struct WorldPos world_pos;
    struct ScreenPos screen_pos;
    struct AABB tile_aabb;
    void *bitmap;

    if (c == '#') {
        bitmap = sc_tile;
    } else if (c == 's') {
        bitmap = sc_soulbooth;
    } else {
        return;
    }

    world_pos = pos_tile_to_world(tile_pos);
    tile_aabb.ax = world_pos.x;
    tile_aabb.ay = world_pos.y;
    tile_aabb.bx = world_pos.x + sc_tile_w;
    tile_aabb.by = world_pos.y + sc_tile_w;
    if (!aabb_aabb(tile_aabb, sc_screen_aabb)) {
        return;
    }

    screen_pos = pos_world_to_screen(world_pos);

    draw_bitmap(
        bitmap,
        screen_pos.x + sc_tile_w / 2,
        screen_pos.y + sc_tile_w / 2,
        0.0);
}

static int lvl_load_read_line(FILE *f, char **buffer, int *length)
{
    int c;
    struct {
        char *data;
        int size, cap;
    } result = { NULL, 0, 0 };

    while ((c = fgetc(f)) != '\n' && c != EOF) {
        ARRAY_APPEND(result, c);
    }

    *length = result.size;
    *buffer = malloc(result.size);
    if (!(*buffer)) {
        DIAG_ERROR("Allocation failure.");
        exit(1);
    }
    memcpy(*buffer, result.data, result.size);

    ARRAY_FREE(result);

    return c;
}

void lvl_load(struct Level *lvl, char *filename)
{
    int width, line_length, c, map_size = 0, map_cap;
    char *map = NULL, *line = NULL;
    FILE *in = fopen(filename, "r");

    if (!in) {
        DIAG_ERROR("Failed opening a file \"%s\".", filename);
        exit(1);
    }

    c = lvl_load_read_line(in, &line, &line_length);
    width = line_length;

    map_cap = line_length;
    map = malloc(map_cap);
    if (!map) {
        DIAG_ERROR("Allocation failure.");
        exit(1);
    }
    memcpy(map, line, line_length);
    free(line);
    map_size = map_cap;

    while (c != EOF) {
        c = lvl_load_read_line(in, &line, &line_length);
        if (line_length != 1) {
            map_cap += line_length;
            map = realloc(map, map_cap);
            if (!map) {
                DIAG_ERROR("Allocation failure.");
                exit(1);
            }
            memcpy(map + map_size, line, line_length);
            map_size = map_cap;
        }
        free(line);
    }

    fclose(in);

    lvl->width = width;
    lvl->height = map_size / width;
    lvl->map = map;
    lvl->map_size = map_size;
}

void lvl_unload(struct Level *lvl)
{
    lvl->width = 0;
    lvl->map_size = 0;
    free(lvl->map);
}

void lvl_for_each_tile(struct Level *lvl, void (*f)(struct TilePos, char))
{
    struct TilePos pos;
    char *current = lvl->map;
    for (pos.y = 0; pos.y < lvl->height; ++pos.y) {
        for (pos.x = 0; pos.x < lvl->width; ++pos.x) {
            f(pos, *(current++));
        }
    }
}

int lvl_get_tile(struct Level *lvl, int x, int y)
{
    if (x < 0 || y < 0 || x >= lvl->width || y >= lvl->height) {
        return -1;
    } else {
        return lvl->map[y * lvl->width + x];
    }
}

void lvl_draw(struct Level *lvl)
{
    lvl_for_each_tile(lvl, lvl_draw_tile);
}

struct NodeArray {
    struct TilePos *data;
    int size, cap;
};

/**
 * Fills a node array with nodes for all the horizontal edges in the level.
 * Note that the y coordinates of the edge nodes will be one tile above the
 * edge y coordinates, because the movement graph is plotted above the walkable
 * tiles.
 */
static void lgph_find_platform_edges(
        struct Level *lvl,
        struct NodeArray *result)
{
    int y;
    for (y = 0; y < lvl->height; ++y) {
        int x = 0, x1, x2;
        while (x < lvl->width) {

            struct TilePos tp;

            while (lvl_get_tile(lvl, x, y) != '#' && x < lvl->width) {
                ++x;
            }
            if (x == lvl->width) {
                break;
            }
            x1 = x;

            while (lvl_get_tile(lvl, x, y) != ' ' && x < lvl->width) {
                ++x;
            }
            x2 = x - 1;

            tp.y = y - 1; /* The _nodes_ are 1 tile above the _edges_. */
            tp.x = x1;
            ARRAY_APPEND(*result, tp);
            tp.x = x2;
            ARRAY_APPEND(*result, tp);
        }
    }
}

static void lgph_add_descent(
        struct Level *lvl,
        struct TilePos upper,
        struct TilePos lower,
        struct NodeArray *result)
{
    int i;
    bool replaced = false;

    for (i = 0; i < result->size; i += 2) {

        int y, x1, x2;
        struct TilePos new_pos;

		/* Discard if the edge is not on the same height as the lower point. */
        if (result->data[i].y != lower.y || result->data[i + 1].y != lower.y) {
            continue;
        }

        y = lower.y;
        x1 = min(result->data[i].x, result->data[i + 1].x);
        x2 = max(result->data[i].x, result->data[i + 1].x);

		/* Discard if the edge doesn't include the lower point. */
        if (lower.x <= x1 || lower.x >= x2) {
            continue;
        }

		replaced = true;

		/* Substitute the first point for the upper descent point
         * and the second point for the lower descent point.
		 */
        result->data[i] = upper;
        result->data[i + 1] = lower;

		/* Insert the left point again. */
        new_pos.x = x1;
        new_pos.y = y;
        ARRAY_APPEND(*result, new_pos);

		/* Insert the new middle point twice: once to end first part of the
		 * broken edge and once to begin the second part of the broken edge.
		 */
        new_pos.x = lower.x;
        new_pos.y = y;
        ARRAY_APPEND(*result, new_pos);
        ARRAY_APPEND(*result, new_pos);

		/* Insert the right point again. */
        new_pos.x = x2;
        new_pos.y = y;
        ARRAY_APPEND(*result, new_pos);
    }

	/* If the descent doesn't break any edge in two parts, then let's just
     * insert it.
	 */
    if (!replaced) {
        ARRAY_APPEND(*result, upper);
        ARRAY_APPEND(*result, lower);
    }
}

/**
 * This will analyze all the platforms' edges. If there is another edge below
 * then a descent edge will be created in the graph. This procedure may or may
 * not land the destination node in the middle of another, existing edge. If
 * an edge is discovered below, then it is broken into two parts so that the
 * point of the descent landing is placed in an actual node and not in the
 * middle of an edge.
 */
void lgph_insert_jump_edges(
        struct Level *lvl,
        struct NodeArray *plat,
        struct NodeArray *result)
{
    int i;
    for (i = 0; i < plat->size; ++i) {
        struct TilePos lower, upper = plat->data[i];
        if (lvl_get_tile(lvl, upper.x + 1, upper.y + 1) != '#' &&
            lvl_get_tile(lvl, upper.x + 1, upper.y + 2) != '#' &&
            lvl_get_tile(lvl, upper.x + 1, upper.y + 3) == '#') {
                lower.x = upper.x + 1;
                lower.y = upper.y + 2;
                lgph_add_descent(lvl, upper, lower, result);
        }
        if (lvl_get_tile(lvl, upper.x - 1, upper.y + 1) != '#' &&
            lvl_get_tile(lvl, upper.x - 1, upper.y + 2) != '#' &&
            lvl_get_tile(lvl, upper.x - 1, upper.y + 3) == '#') {
                lower.x = upper.x - 1;
                lower.y = upper.y + 2;
                lgph_add_descent(lvl, upper, lower, result);
        }
    }
}

static struct NodeArray lgph_find_unique_nodes(struct NodeArray *in)
{
    int i, j;
    bool found;
    struct NodeArray uniques = { NULL, 0, 0 };

    for (i = 0; i < in->size; ++i) {
        found = false;
        for (j = 0; j != uniques.size; ++j) {
			if (eq_tilepos(in->data + i, uniques.data + j)) {
				found = true;
				break;
            }
        }
        if (!found) {
            ARRAY_APPEND(uniques, in->data[i]);
        }
    }

    return uniques;
}

static struct LvlAdj *lgph_find_adjacency(
        struct TilePos n,
        struct NodeArray *uniques,
        struct NodeArray *all)
{
    int i;

    struct {
        struct LvlAdj *data;
        int size, cap;
    } result = { NULL, 0, 0 };

    struct LvlAdj adj;

	/* For each edge in the graph. */
    for (i = 0; i < all->size; i += 2) {

        int j;
		struct TilePos
			*first = all->data + i,
			*second = all->data + i + 1;

		/* Analyze the n node against the current edge. */
		bool is_first = eq_tilepos(&n, first);
        bool is_second = eq_tilepos(&n, second);
        bool is_vertical = first->y != second->y;

        if (is_first && !is_second) {
			/* Lookup the second node and set as the n's adjacency. */
            for (j = 0; j < uniques->size; ++j) {
				struct TilePos *unique = uniques->data + j;
                if (eq_tilepos(second, unique)) {
					adj.neighbor = j;
					adj.type = is_vertical ? LVL_ADJ_JUMP : LVL_ADJ_WALK;
					ARRAY_APPEND(result, adj);
					break;
                }
            }
        } else if (!is_first && is_second) {
			/* Lookup the first node and set as the n's adjacency. */
            for (j = 0; j < uniques->size; ++j) {
				struct TilePos *unique = uniques->data + j;
                if (eq_tilepos(first, unique)) {
					adj.neighbor = j;
					adj.type = is_vertical ? LVL_ADJ_JUMP : LVL_ADJ_WALK;
					ARRAY_APPEND(result, adj);
					break;
                }
            }
        } else {
            /* NULL; */
        }
    }

	/* Add an awkward plug; WTF? :o */
    adj.neighbor = -1;
    ARRAY_APPEND(result, adj);

    return result.data;
}

static void lgph_dijkstra(
        struct LvlGraph *lgph, struct TilePos src_pos, struct TilePos dst_pos,
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
        struct LvlAdj *adj;
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

struct LvlGraph lgph_init(struct Level *lvl)
{
    int i;
    struct LvlGraph result;
    struct NodeArray uniques = { 0, }, plat_nodes = { 0, }, all_nodes = { 0, };
    struct LvlAdj **adjacency = NULL;

    lgph_find_platform_edges(lvl, &all_nodes);

    ARRAY_COPY(plat_nodes, all_nodes);
    lgph_insert_jump_edges(lvl, &plat_nodes, &all_nodes);
    /* TODO: Also generate jumps over ledges (?) */

    uniques = lgph_find_unique_nodes(&all_nodes);

    adjacency = malloc(uniques.size * sizeof(*adjacency));
    if (!adjacency) {
        DIAG_ERROR("Allocation failure.");
        exit(1);
    }
    for (i = 0; i < uniques.size; ++i) {
        adjacency[i] = lgph_find_adjacency(
            uniques.data[i], &uniques, &all_nodes);
    }

    ARRAY_FREE(all_nodes);

    result.nodes_count = uniques.size;
    result.nodes = uniques.data;
    result.adjacency = adjacency;

    return result;
}

void lgph_deinit(struct LvlGraph *lgph)
{
    int i;
    free(lgph->nodes);
    for (i = 0; i < lgph->nodes_count; ++i) {
        free(lgph->adjacency[i]);
    }
    free(lgph->adjacency);
}

int lgph_find_index(struct LvlGraph *lgph, struct TilePos pos)
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

int lgph_find_farthest(struct LvlGraph *lgph, struct TilePos bad)
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
        struct LvlGraph *lgph, struct TilePos src, struct TilePos bad,
        struct TilePos **points, int *points_count)
{
    int dst = lgph_find_farthest(lgph, bad);
    lgph_dijkstra(lgph, src, lgph->nodes[dst], points, points_count);
}

void lgph_random_path(
        struct LvlGraph *lgph, struct TilePos src,
        struct TilePos **points, int *points_count)
{
    int dst = rnd_uniform_rng_i(0, lgph->nodes_count - 1);
    lgph_dijkstra(lgph, src, lgph->nodes[dst], points, points_count);
}
