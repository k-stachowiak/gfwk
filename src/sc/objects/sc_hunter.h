/* Copyright (C) 2014 Krzysztof Stachowiak */

#ifndef HUNTER_H
#define HUNTER_H

#include <stdbool.h>

#include "cmp_appr.h"
#include "cmp_ori.h"
#include "cmp_drv.h"
#include "cmp_pain.h"
#include "cmp_shape.h"

enum HunterState {
	HUNTER_STATE_STAND_RIGHT,
	HUNTER_STATE_STAND_LEFT,
	HUNTER_STATE_WALK_RIGHT,
	HUNTER_STATE_WALK_LEFT
};

struct Hunter {
	long id;
	bool has_soul;
	struct CmpDrv drv;
	struct CmpAppr appr;
	struct CmpOri ori;
	struct CmpPain pain;
	struct CmpShape shape;
    double aim_angle;
    double box_w, box_h;
    int inx;
    bool jump_req;
    bool standing;
};

void hunter_init(struct Hunter *hunter, long id);
void hunter_deinit(struct Hunter *hunter);

void hunter_set_state(struct CmpAppr *appr, enum HunterState state);
enum HunterState hunter_get_state(struct CmpAppr *appr);

#endif
