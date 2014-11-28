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
#include "sc_graph.h"

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

/**
 * Fills a node array with nodes for all the horizontal edges in the level.
 * Note that the y coordinates of the edge nodes will be one tile above the
 * edge y coordinates, because the movement graph is plotted above the walkable
 * tiles.
 */
static void lvl_init_graph_find_platform_edges(
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

static void lgph_init_graph_add_descent(
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
static void lvl_init_graph_insert_jump_edges(
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
                lgph_init_graph_add_descent(lvl, upper, lower, result);
        }
        if (lvl_get_tile(lvl, upper.x - 1, upper.y + 1) != '#' &&
            lvl_get_tile(lvl, upper.x - 1, upper.y + 2) != '#' &&
            lvl_get_tile(lvl, upper.x - 1, upper.y + 3) == '#') {
                lower.x = upper.x - 1;
                lower.y = upper.y + 2;
                lgph_init_graph_add_descent(lvl, upper, lower, result);
        }
    }
}

static struct NodeArray lvl_init_graph_find_unique_nodes(struct NodeArray *in)
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

static struct Adj *lvl_init_graph_find_adjacency(
        struct TilePos n,
        struct NodeArray *uniques,
        struct NodeArray *all)
{
    int i;

    struct {
        struct Adj *data;
        int size, cap;
    } result = { NULL, 0, 0 };

    struct Adj adj;

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
					adj.type = is_vertical ? ADJ_JUMP : ADJ_WALK;
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
					adj.type = is_vertical ? ADJ_JUMP : ADJ_WALK;
					ARRAY_APPEND(result, adj);
					break;
                }
            }
        } else {
            /* NULL; */
        }
    }

    /* Terminating element for the sequence. */
    adj.neighbor = -1;
    ARRAY_APPEND(result, adj);

    return result.data;
}

struct Graph lvl_init_graph(struct Level *lvl)
{
    int i;
    struct Graph result;
    struct NodeArray uniques = { 0, }, plat_nodes = { 0, }, all_nodes = { 0, };
    struct Adj **adjacency = NULL;

    lvl_init_graph_find_platform_edges(lvl, &all_nodes);

    ARRAY_COPY(plat_nodes, all_nodes);
    lvl_init_graph_insert_jump_edges(lvl, &plat_nodes, &all_nodes);

    uniques = lvl_init_graph_find_unique_nodes(&all_nodes);

    adjacency = malloc(uniques.size * sizeof(*adjacency));
    if (!adjacency) {
        DIAG_ERROR("Allocation failure.");
        exit(1);
    }
    for (i = 0; i < uniques.size; ++i) {
        adjacency[i] = lvl_init_graph_find_adjacency(
            uniques.data[i], &uniques, &all_nodes);
    }

    ARRAY_FREE(all_nodes);

    lgph_init(&result, uniques.size, uniques.data, adjacency);

    return result;
}

