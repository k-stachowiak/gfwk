/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <stdio.h>
#include <string.h>

#include <allegro5/allegro_primitives.h>

#include "draw.h"
#include "array.h"
#include "diagnostics.h"
#include "sc_level.h"

static void lvl_draw_tile(struct TilePos tile_pos, char c)
{
    struct WorldPos world_pos;
    struct ScreenPos screen_pos;
    struct AABB tile_aabb;

    if (c == ' ') {
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
        sc_rock_tile,
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
        DIAG_ERROR("Allocation failure.\n");
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
        DIAG_ERROR("Failed opening a file \"%s\".\n", filename);
        exit(1);
    }

    c = lvl_load_read_line(in, &line, &line_length);
    width = line_length;

    map_cap = line_length;
    map = malloc(map_cap);
    if (!map) {
        DIAG_ERROR("Allocation failure.\n");
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
                DIAG_ERROR("Allocation failure.\n");
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

static void lgph_find_vert_nodes(struct Level *lvl, struct NodeArray *ns)
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

            tp.y = y - 1;
            tp.x = x1;
            ARRAY_APPEND(*ns, tp);
            tp.x = x2;
            ARRAY_APPEND(*ns, tp);
        }
    }
}

static struct NodeArray lgph_find_unique_nodes(struct NodeArray *in)
{
    int i, j;
    bool found;
    struct NodeArray uniques = { NULL, 0, 0 };

    for (i = 0; i < in->size; i += 2) {

        found = false;
        for (j = 0; j != uniques.size; ++j) {
            if (in->data[i].x == uniques.data[j].x &&
                in->data[i].y == uniques.data[j].y) {
                    found = true;
                    break;
            }
        }
        if (!found) {
            ARRAY_APPEND(uniques, in->data[i]);
        }

        found = false;
        for (j = 0; j != uniques.size; ++j) {
            if (in->data[i + 1].x == uniques.data[j].x &&
                in->data[i + 1].y == uniques.data[j].y) {
                    found = true;
                    break;
            }
        }
        if (!found) {
            ARRAY_APPEND(uniques, in->data[i + 1]);
        }
    }

    return uniques;
}

static int *lgph_find_adjacency(
        struct TilePos n,
        struct NodeArray *unique,
        struct NodeArray *all)
{
    int i;
    bool found, is_first, is_second;
    struct {
        int *data;
        int size, cap;
    } result = { NULL, 0, 0 };

    for (i = 0; i < all->size; i += 2) {

        int j;
        is_first = n.x == all->data[i].x && n.y == all->data[i].y;
        is_second = n.x == all->data[i + 1].x && n.y == all->data[i + 1].y;

        if (is_first && !is_second) {
            found = false;
            for (j = 0; j < unique->size; ++j) {
                if (all->data[i + 1].x == unique->data[j].x &&
                    all->data[i + 1].y == unique->data[j].y) {
                        ARRAY_APPEND(result, j);
                        found = true;
                        break;
                }
            }
            if (!found) {
                DIAG_ERROR("Data corruption. Neighbor not found.\n");
                exit(1);
            }
        } else if (!is_first && is_second) {
            found = false;
            for (j = 0; j < unique->size; ++j) {
                if (all->data[i].x == unique->data[j].x &&
                    all->data[i].y == unique->data[j].y) {
                        ARRAY_APPEND(result, j);
                        found = true;
                        break;
                }
            }
            if (!found) {
                DIAG_ERROR("Data corruption. Neighbor not found.\n");
                exit(1);
            }
        } else {
            DIAG_DEBUG("Pair in the same tile found.\n");
        }
    }

    ARRAY_APPEND(result, -1);

    return result.data;
}

struct LvlGraph lgph_init(struct Level *lvl)
{
    /*
     * [v] Find and draw all the basic vertical edges (w. nodes)
     * [ ] Find and draw all the vertical edges (w. connectors to the upper edge(s))
     * [ ] Break the vertical edges with horizontal ones.
     */

    int i;
    struct LvlGraph result;
    struct NodeArray uniques, nodes = { NULL, 0, 0 };
    int **adjacency;

    lgph_find_vert_nodes(lvl, &nodes);
    /* ... */

    uniques = lgph_find_unique_nodes(&nodes);

    adjacency = malloc(uniques.size * sizeof(*adjacency));
    if (!adjacency) {
        DIAG_ERROR("Allocation failure.\n");
        exit(1);
    }
    for (i = 0; i < uniques.size; ++i) {
        adjacency[i] = lgph_find_adjacency(uniques.data[i], &uniques, &nodes);
    }

    ARRAY_FREE(nodes);

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
