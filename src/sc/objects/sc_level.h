/* Copyright (C) 2014 Krzysztof Stachowiak */

#ifndef SC_LEVEL_H
#define SC_LEVEL_H

struct Level {
    int width, height;
    char *map;
    int map_size;
};

void lvl_load(struct Level *lvl, char *filename);
void lvl_unload(struct Level *lvl);
void lvl_for_each_tile(struct Level *lvl, void (*f)(struct TilePos, char));
int lvl_get_tile(struct Level *lvl, int x, int y);

struct Graph lvl_init_graph(struct Level *lvl);

#endif
