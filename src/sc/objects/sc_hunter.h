/* Copyright (C) 2014 Krzysztof Stachowiak */

#ifndef HUNTER_H
#define HUNTER_H

#include <stdbool.h>

#include "cmp_appr.h"
#include "cmp_ori.h"
#include "cmp_drv.h"
#include "cmp_pain.h"
#include "cmp_shape.h"

enum HunterAppearance {
	HUNTER_APPR_STAND_LEFT,
	HUNTER_APPR_STAND_RIGHT,
	HUNTER_APPR_WALK_LEFT,
	HUNTER_APPR_WALK_RIGHT
};

struct Hunter {
	struct CmpApprStaticSprite appr_stand_right;
	struct CmpApprStaticSprite appr_stand_left;
	struct CmpApprAnimSprite appr_walk_right;
	struct CmpApprAnimSprite appr_walk_left;
	struct CmpAppr *appr_array[4];

	long id;
	bool has_soul;
	struct CmpDrvPlatform drv;
	struct CmpApprProxy appr;
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

#endif
