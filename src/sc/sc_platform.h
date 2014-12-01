/* Copyright (C) 2014 Krzysztof Stachowiak */

#ifndef SC_PLATFORM_H
#define SC_PLATFORM_H

#include "sc_data.h"
#include "sc_hunter.h"
#include "sc_level.h"

void platform_draw_debug(void);
void platform_collide(struct Hunter *hunter, struct Level *lvl);

#endif
