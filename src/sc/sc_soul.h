/* Copyright (C) 2014 Krzysztof Stachowiak */

#ifndef SC_SOUL_H
#define SC_SOUL_H

#include "sc_data.h"
#include "sc_level.h"

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

struct Soul {
	struct CmpDrv *drv;

    struct CmpApprStaticSprite appr_stand_right;
	struct CmpApprStaticSprite appr_stand_left;
    struct CmpApprAnimSprite appr_walk_right;
	struct CmpApprAnimSprite appr_walk_left;
	struct CmpApprStaticSprite appr_caught;
	struct CmpAppr *appr_array[5];

    struct CmpApprProxy appr;
	struct CmpOri ori;
	struct CmpPain pain;
	struct CmpAiSoul ai;

    double box_w, box_h;
};

void soul_init(struct Soul *soul, struct Graph *lgph, struct TilePos tp);
void soul_deinit(struct Soul *soul);
void soul_tick(struct Soul *soul, struct CmpAiTacticalStatus *ts, double dt);
void soul_draw(struct Soul *soul);

#endif
