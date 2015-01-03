/* Copyright (C) 2014 Krzysztof Stachowiak */

#ifndef SC_SOUL_H
#define SC_SOUL_H

#include "cmp_appr.h"
#include "cmp_ori.h"
#include "cmp_pain.h"
#include "cmp_ai.h"
#include "cmp_shape.h"

struct Soul {
	long id;
	struct CmpDrv drv;
    struct CmpAppr appr;
	struct CmpOri ori;
	struct CmpPain pain;
	struct CmpAi ai;
	struct CmpShape shape;

	double health;
    double box_w, box_h;
};

void soul_init(struct Soul *soul, long id, struct Graph *lgph, struct TilePos tp);
void soul_deinit(struct Soul *soul);

void soul_set_appr_stand_right(struct CmpAppr *appr);
void soul_set_appr_stand_left(struct CmpAppr *appr);
void soul_set_appr_walk_right(struct CmpAppr *appr);
void soul_set_appr_walk_left(struct CmpAppr *appr);
void soul_set_appr_caught(struct CmpAppr *appr);

void soul_set_drv_stand(struct CmpDrv *drv);
void soul_set_drv_walk(struct CmpDrv *drv);

#endif
