/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <math.h>

#include "sc_arrow.h"
#include "sc_data.h"

#include "cmp_appr.h"
#include "cmp_drv.h"

#include "cmp_operations.h"

void arrow_init(struct Arrow *arrow, double x, double y, double angle)
{
    static double vel = 600.0;
    arrow->appr = cmp_appr_static_sprite_create(sc_arrow_bitmap);
    arrow->drv = cmp_drv_ballistic_create(true, cos(angle) * vel, sin(angle) * vel);
	
	cmp_pain_init(&arrow->pain, PT_ARROW);
	cmp_ori_init(&arrow->ori, x, y, angle);
}

void arrow_deinit(struct Arrow *arrow)
{
    arrow->appr->free(arrow->appr);
    arrow->drv->free(arrow->drv);

    cmp_pain_deinit(&arrow->pain);
	cmp_ori_deinit(&arrow->ori);
}

bool arrow_tick(struct Arrow *arrow, double dt)
{
    static int margin = 20;

    struct PosRot pr;
    struct WorldPos wp;
    struct ScreenPos sp;

    arrow->drv->update(arrow->drv, dt);
    cmp_drive(&arrow->ori, arrow->drv, dt);

    pr = cmp_ori_get(&arrow->ori);
    wp.x = pr.x;
    wp.y = pr.y;
    sp = pos_world_to_screen(wp);

    if (sp.x < margin || sp.x > (sc_screen_w - margin) ||
        sp.y < margin || sp.y > (sc_screen_h - margin)) {
        return false;
    } else {
        return true;
    }
}

