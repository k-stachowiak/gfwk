/* Copyright (C) 2014 Krzysztof Stachowiak */

#ifndef SC_SOUL_H
#define SC_SOUL_H

#include "cmp_appr.h"
#include "cmp_ori.h"
#include "cmp_pain.h"
#include "cmp_ai.h"

struct CmpAiTacticalStatus;

enum SoulAppearance {
	SOUL_APPR_STAND_LEFT,
	SOUL_APPR_STAND_RIGHT,
	SOUL_APPR_WALK_LEFT,
	SOUL_APPR_WALK_RIGHT,
	SOUL_APPR_CAUGHT,
};

enum SoulDriver {
	SOUL_DRV_STAND,
	SOUL_DRV_WALK
};

struct Soul {
    struct CmpApprStaticSprite appr_stand_right;
	struct CmpApprStaticSprite appr_stand_left;
    struct CmpApprAnimSprite appr_walk_right;
	struct CmpApprAnimSprite appr_walk_left;
	struct CmpApprStaticSprite appr_caught;
	struct CmpAppr *appr_array[5];

	struct CmpDrvLinear drv_stand;
	struct CmpDrvWaypoint drv_walk;
	struct CmpDrv *drv_array[2];

	long id;
	struct CmpDrvProxy drv;
    struct CmpApprProxy appr;
	struct CmpOri ori;
	struct CmpPain pain;
	struct CmpAiSoul ai;

	double health;
    double box_w, box_h;
};

void soul_init(struct Soul *soul, long id, struct Graph *lgph, struct TilePos tp);
void soul_deinit(struct Soul *soul);

#endif
