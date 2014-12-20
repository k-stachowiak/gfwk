/* Copyright (C) 2014 Krzysztof Stachowiak */

#ifndef SC_PAIN_H
#define SC_PAIN_H

void pain_draw_debug(void);

void pain_tick(
        struct Arrow *arrows, int arrows_count,
        struct Soul *soul);

#endif
