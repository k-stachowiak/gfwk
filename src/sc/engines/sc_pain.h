/* Copyright (C) 2014 Krzysztof Stachowiak */

#ifndef SC_PAIN_H
#define SC_PAIN_H

typedef void(*PainCallback)(long, void*);

void sc_pain_init(void);
void sc_pain_deinit(void);

void sc_pain_draw_debug(void);

void sc_pain_tick(
		struct ArrowArray *arrows,
		struct ArrowArray *arrows_stuck,
		struct Soul *soul);

void sc_pain_callback_register(long id, void *data, PainCallback callback);

#endif
