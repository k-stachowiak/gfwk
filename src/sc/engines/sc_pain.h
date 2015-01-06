/* Copyright (C) 2014 Krzysztof Stachowiak */

#ifndef SC_PAIN_H
#define SC_PAIN_H

typedef void(*PainCallbackId)(long, PainType, void*);
typedef void(*PainCallbackType)(PainType, long, PainType, void*);

void sc_pain_init(void);
void sc_pain_deinit(void);

void sc_pain_draw_debug(void);

void sc_pain_tick(
		struct Hunter *hunter,
		struct ArrowArray *arrows,
		struct SoulArray *souls,
		struct BoothArray *booths);

void sc_pain_callback_id_register(long id, void *data, PainCallbackId callback);
void sc_pain_callback_type_register(PainType type, void *data, PainCallbackType callback);

#endif
