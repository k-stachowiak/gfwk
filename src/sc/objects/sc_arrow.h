/* Copyright (C) 2014 Krzysztof Stachowiak */

#ifndef SC_ARROW_H
#define AC_ARROW_H

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

#endif