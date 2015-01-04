/* Copyright (C) 2014 Krzysztof Stachowiak */

#ifndef SC_SOUL_H
#define SC_SOUL_H

#include "cmp_appr.h"
#include "cmp_ori.h"
#include "cmp_pain.h"
#include "cmp_ai.h"
#include "cmp_shape.h"

enum SoulDir {
	SOUL_DIR_RIGHT,
	SOUL_DIR_LEFT,
};

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

void soul_start_walking(
		struct CmpDrv *drv,
		double *points, int points_count,
		CmpDrvCallback on_end, void *data);

bool soul_walking(struct CmpAppr *appr);
enum SoulDir soul_walking_dir(struct CmpAppr *appr);
void soul_update_walking(struct CmpAppr *appr, enum SoulDir dir);
void soul_knock_out(struct CmpAppr *appr, struct CmpDrv *drv);

void soul_set_state(struct CmpAppr *appr, struct CmpDrv *drv, enum SoulState state);
enum SoulState soul_get_state(struct CmpAppr *appr, struct CmpDrv *drv);

#endif
