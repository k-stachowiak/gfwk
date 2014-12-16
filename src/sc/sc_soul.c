/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <stdlib.h>

#include "cmp_appr.h"
#include "cmp_drv.h"
#include "cmp_pain.h"
#include "cmp_operations.h"
#include "sc_soul.h"

#include "diagnostics.h"
#include "resources.h"

void soul_init(struct Soul *soul, struct Graph *lgph, struct TilePos soul_tp)
{
    struct WorldPos wp = pos_tile_to_world_ground(soul_tp);

    soul->appr_walk_right = cmp_appr_anim_sprite_create(sc_soul_walk_right_common, 2, -1);
    soul->appr_walk_left = cmp_appr_anim_sprite_create(sc_soul_walk_left_common, 2, -1);
    soul->appr_stand_right = cmp_appr_static_sprite_create(sc_soul_stand_right);
    soul->appr_stand_left = cmp_appr_static_sprite_create(sc_soul_stand_left);
    soul->appr_caught = cmp_appr_static_sprite_create(sc_soul_caught);

    soul->appr = soul->appr_walk_right;
    soul->drv = cmp_drv_waypoint_create(150.0);

	cmp_ori_init(&soul->ori, wp.x, wp.y, 0.0);
	cmp_pain_init(&soul->pain, PT_SOUL);
	cmp_ai_soul_init(&soul->ai, lgph, &soul->ori, soul->drv);

    soul->box_w = 30.0;
    soul->box_h = 60.0;
}

void soul_deinit(struct Soul *soul)
{
    soul->appr_stand_right->free(soul->appr_stand_right);
    soul->appr_stand_left->free(soul->appr_stand_left);
    soul->appr_walk_right->free(soul->appr_walk_right);
    soul->appr_walk_left->free(soul->appr_walk_left);
    soul->appr_caught->free(soul->appr_caught);

    soul->drv->free(soul->drv);

	cmp_ai_soul_deinit(&soul->ai);
	cmp_pain_deinit(&soul->pain);
	cmp_ori_deinit(&soul->ori);
}

void soul_tick(struct Soul *soul, struct CmpAiTacticalStatus *ts, double dt)
{
    struct Vel vel;

	soul->appr->update(soul->appr, dt);
	soul->drv->update(soul->drv, dt);

    soul->ai.base.update(CMP_AI(&soul->ai), &soul->ori, soul->drv, ts, dt);

    vel = soul->drv->vel(soul->drv);

    if (vel.vx > 0) {
        soul->appr = soul->appr_walk_right;
    } else if (vel.vx < 0) {
        soul->appr = soul->appr_walk_left;
    }

	cmp_think(CMP_AI(&soul->ai));
    cmp_drive(&soul->ori, soul->drv, dt);
}

void soul_draw(struct Soul *soul)
{
    struct WorldPos zero_wp = { 0.0, 0.0 };
    struct ScreenPos zero_sp = pos_world_to_screen(zero_wp);
    cmp_draw(&soul->ori, soul->appr, -zero_sp.x, -zero_sp.y);
}

