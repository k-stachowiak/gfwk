/* Copyright (C) 2014 Krzysztof Stachowiak */

#ifndef SC_DRAW_H
#define SC_DRAW_H

void sc_draw_level(struct Level *lvl);
void sc_draw_hunter(struct Hunter *hunter);
void sc_draw_soul(struct Soul *soul);
void sc_draw_soul_dbg(struct Soul *soul);
void sc_draw_arrows(struct ArrowArray *aa);
void sc_draw_graph_dbg(struct Graph *lgph);

#endif