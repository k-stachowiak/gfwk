/* Copyright (C) 2014 Krzysztof Stachowiak */

#ifndef CMP_OPERATIONS_H
#define CMP_OPERATIONS_H

#include "cmp_appr.h"
#include "cmp_ori.h"
#include "cmp_drv.h"
#include "cmp_ai.h"
#include "cmp_pain.h"

void cmp_draw(struct CmpOri *ori, struct CmpAppr *appr, double vsx, double vsy);
void cmp_drive(struct CmpOri *ori, struct CmpDrv *drv, double dt);
void cmp_deal_pain(struct CmpPain *x, struct CmpPain *y);

#endif
