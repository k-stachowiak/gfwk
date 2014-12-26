/* Copyright (C) 2014 Krzysztof Stachowiak */

#ifndef SC_TICK_H
#define SC_TICK_H

void sc_tick_camera(double dt);
void sc_tick_arrows_regular(struct ArrowArray *array, double dt);
void sc_tick_arrows_stuck(struct ArrowArray *array, double dt);
void sc_tick_hunter(struct Hunter *hunter, double dt);
void sc_tick_souls(struct SoulArray *souls, struct CmpAiTacticalStatus *ts, double dt);

#endif