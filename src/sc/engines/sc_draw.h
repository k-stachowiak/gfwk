/* Copyright (C) 2014,2015 Krzysztof Stachowiak */

#ifndef SC_DRAW_H
#define SC_DRAW_H

void sc_draw_level(struct Level *lvl);
void sc_draw_hunter(struct Hunter *hunter);
void sc_draw_souls(struct SoulArray *souls);
void sc_draw_souls_dbg(struct SoulArray *souls);
void sc_draw_arrows(struct ArrowArray *aa);
void sc_draw_booths(struct BoothArray *booths);
void sc_draw_graph_dbg(struct Graph *lgph);

#endif