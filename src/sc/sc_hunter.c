/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <allegro5/allegro_primitives.h>

#include "cmp_appr.h"
#include "cmp_ori.h"
#include "cmp_drv.h"
#include "cmp_operations.h"
#include "sc_hunter.h"

static double center_shift_x = 20.0;
static double center_shift_y = 0.0;

void hunter_init(struct Hunter *hunter, void *stand_bitmap)
{
    hunter->appr = cmp_appr_create_static_sprite(
            center_shift_x, center_shift_y, stand_bitmap);

    hunter->ori = cmp_ori_create(0.0, 0.0, 0.0);
    hunter->drv = cmp_drv_create_platform(
            &hunter->inx,
            &hunter->jump_req,
            &hunter->standing);

    hunter->box_w = 30.0;
    hunter->box_h = 60.0;

    hunter->jump_req = false;
    hunter->standing = false;
}

void hunter_tick(struct Hunter *hunter, double dt)
{
    cmp_drive(hunter->ori, hunter->drv, dt);
}

void hunter_draw(struct Hunter *hunter)
{
    struct PosRot hunter_pr = cmp_ori_get(hunter->ori);
    struct WorldPos hunter_wp = { hunter_pr.x, hunter_pr.y };
    struct ScreenPos hunter_sp = pos_world_to_screen(hunter_wp);

    struct WorldPos zero_wp = { 0.0, 0.0 };
    struct ScreenPos zero_sp = pos_world_to_screen(zero_wp);

    cmp_draw(hunter->ori, hunter->appr, -zero_sp.x, -zero_sp.y);

    /* DEBUG */
    double x1, y1, x2, y2;
    aabb_to_screen(sc_last_hunter_aabb, &x1, &y1, &x2, &y2);
    al_draw_rectangle(x1, y1, x2, y2, al_map_rgb_f(1, 0, 1), 1.0);
    al_draw_filled_circle(hunter_sp.x, hunter_sp.y, 5, al_map_rgb_f(0, 0, 1));
}

