/* Copyright (C) 2015 Krzysztof Stachowiak */

#ifndef SC_BOOTH_H
#define SC_BOOTH_H

#include "cmp_appr.h"
#include "cmp_pain.h"
#include "cmp_ori.h"
#include "cmp_shape.h"

struct Booth {
	long id;
	struct CmpAppr appr;
	struct CmpPain pain;
	struct CmpOri ori;
	struct CmpShape shape;
};

void booth_init(struct Booth *booth, long id, struct TilePos tile_pos);
void booth_deinit(struct Booth *booth);

#endif