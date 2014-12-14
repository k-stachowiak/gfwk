/* Copyright (C) 2014 Krzysztof Stachowiak */

#ifndef SC_PAIN_H
#define SC_PAIN_H

#include "sc_arrow.h"
#include "sc_soul.h"

void pain_draw_debug(void);
void pain_tick(
        struct Arrow *arrows, int arrows_count,
        struct Soul *soul);

#endif
