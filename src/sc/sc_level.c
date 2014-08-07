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
        0.0, 0.0,
        0.0);

    /* DEBUG */
    double x1, y1, x2, y2;
    aabb_to_screen(tile_aabb, &x1, &y1, &x2, &y2);
    al_draw_rectangle(x1, y1, x2, y2, al_map_rgb_f(1, 0, 1), 1.0);
    al_draw_filled_circle(screen_pos.x, screen_pos.y, 5, al_map_rgb_f(0, 0, 1));
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
    }

    c = lvl_load_read_line(in, &line, &line_length);
    width = line_length;

    map_cap = line_length;
    map = malloc(map_cap);
    if (!map) {
        DIAG_ERROR("Allocation failure.\n");
    }
    memcpy(map, line, line_length);
    free(line);
    map_size = map_cap;

    while (c != EOF) {
        c = lvl_load_read_line(in, &line, &line_length);
        if (line_length != 1 && c != EOF) {
            map_cap += line_length;
            map = realloc(map, map_cap);
            if (!map) {
                DIAG_ERROR("Allocation failure.\n");
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
