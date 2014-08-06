/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <math.h>

#include "cmp_appr.h"
#include "cmp_ori.h"
#include "cmp_drv.h"
#include "draw.h"

void cmp_draw(struct CmpOri *ori, struct CmpAppr *appr)
{
    struct PosRot pr = cmp_ori_get(ori);
    void *bitmap = cmp_appr_bitmap(appr);
    draw_bitmap(bitmap, pr.x, pr.y, pr.theta);
}

void cmp_drive(struct CmpOri *ori, struct CmpDrv *drv, double dt)
{
    struct Vel vel = cmp_drv_vel(drv);

    if (drv->affect_rot && (vel.vx != 0 || vel.vy != 0)) {
        double new_theta = atan2(vel.vy, vel.vx);
        ori->pr_buffer[ori->prb_top].theta = new_theta;
    }

    cmp_ori_shift_rotate(ori, vel.vx * dt, vel.vy * dt, vel.vtheta * dt);
}
