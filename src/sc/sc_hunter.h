/* Copyright (C) 2014 Krzysztof Stachowiak */

#ifndef HUNTER_H
#define HUNTER_H

#include <stdbool.h>

#include "sc_data.h"

#include "cmp_appr.h"
#include "cmp_ori.h"

enum HunterAppearance {
	HUNTER_APPR_STAND_LEFT,
	HUNTER_APPR_STAND_RIGHT,
	HUNTER_APPR_WALK_LEFT,
	HUNTER_APPR_WALK_RIGHT
};

struct Hunter {
    struct CmpDrv *drv;

	struct CmpApprStaticSprite appr_stand_right;
	struct CmpApprStaticSprite appr_stand_left;
	struct CmpApprAnimSprite appr_walk_right;
	struct CmpApprAnimSprite appr_walk_left;
	struct CmpAppr *appr_array[4];

	struct CmpApprProxy appr;
	struct CmpOri ori;

    double aim_angle;
    double box_w, box_h;
    int inx;
    bool jump_req;
    bool standing;
};

void hunter_init(struct Hunter *hunter);
void hunter_deinit(struct Hunter *hunter);
void hunter_tick(struct Hunter *hunter, double dt);
void hunter_draw(struct Hunter *hunter);

#endif
