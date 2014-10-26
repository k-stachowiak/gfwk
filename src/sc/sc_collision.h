/* Copyright (C) 2014 Krzysztof Stachowiak */

#ifndef SC_COLLISION_H
#define SC_COLLISION_H

#include "sc_level.h"
#include "sc_hunter.h"

void col_handle_all(struct Hunter *hunter, struct Level *lvl);
void col_draw_last(void);

#endif
