/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <math.h>

#include "diagnostics.h"
#include "draw.h"
#include "cmp_operations.h"

void cmp_draw(struct CmpOri *ori, struct CmpAppr *appr, double vsx, double vsy)
{
    struct PosRot pr = cmp_ori_get(ori);
    void *bitmap = appr->bitmap(appr);
    draw_bitmap(bitmap, pr.x - vsx, pr.y - vsy, pr.theta);
}

void cmp_drive(struct CmpOri *ori, struct CmpDrv *drv, double dt)
{
    struct Vel vel;

    vel = drv->vel(drv);

    if (drv->affect_rot && (vel.vx != 0 || vel.vy != 0)) {
        double new_theta = atan2(vel.vy, vel.vx);
        ori->current.theta = new_theta;
    }

    cmp_ori_shift_rotate(ori, vel.vx * dt, vel.vy * dt, vel.vtheta * dt);
}

void cmp_think(struct CmpAi *ai)
{
    (void)ai;
}

void cmp_deal_pain(struct CmpPain *x, struct CmpPain *y)
{
    cmp_pain_queue_push(x, y->type);
    cmp_pain_queue_push(y, x->type);
    DIAG_TRACE("pejn");
}
