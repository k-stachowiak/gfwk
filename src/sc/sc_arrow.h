/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <stdbool.h>

#include "cmp_drv.h"
#include "cmp_ori.h"
#include "cmp_pain.h"
#include "cmp_appr.h"

struct Arrow {
    struct CmpDrvBallistic drv;
	struct CmpApprStaticSprite appr;
    struct CmpPain pain;
	struct CmpOri ori;

    double timer; /* multi-purpose ill-designed helper counter. */
};

void arrow_init(struct Arrow *arrow, double x, double y, double angle);
void arrow_deinit(struct Arrow *arrow);
bool arrow_tick(struct Arrow *arrow, double dt);

