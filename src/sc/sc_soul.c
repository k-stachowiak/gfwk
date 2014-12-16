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

	soul->drv = cmp_drv_waypoint_create(150.0);

	cmp_appr_anim_sprite_init(&soul->appr_walk_right, sc_soul_walk_right_common, 2, -1);
	cmp_appr_anim_sprite_init(&soul->appr_walk_left, sc_soul_walk_left_common, 2, -1);
	cmp_appr_static_sprite_init(&soul->appr_stand_right, sc_soul_stand_right);
	cmp_appr_static_sprite_init(&soul->appr_stand_left, sc_soul_stand_left);
	cmp_appr_static_sprite_init(&soul->appr_caught, sc_soul_caught);

	soul->appr_array[SOUL_APPR_STAND_LEFT] = CMP_APPR(&soul->appr_stand_left);
	soul->appr_array[SOUL_APPR_STAND_RIGHT] = CMP_APPR(&soul->appr_stand_right);
	soul->appr_array[SOUL_APPR_WALK_LEFT] = CMP_APPR(&soul->appr_walk_left);
	soul->appr_array[SOUL_APPR_WALK_RIGHT] = CMP_APPR(&soul->appr_walk_right);
	soul->appr_array[SOUL_APPR_CAUGHT] = CMP_APPR(&soul->appr_caught);

	cmp_appr_proxy_init(&soul->appr, soul->appr_array, 5, SOUL_APPR_WALK_RIGHT);

	cmp_ori_init(&soul->ori, wp.x, wp.y, 0.0);
	cmp_pain_init(&soul->pain, PT_SOUL);
	cmp_ai_soul_init(&soul->ai, lgph, &soul->ori, soul->drv);

    soul->box_w = 30.0;
    soul->box_h = 60.0;
}

void soul_deinit(struct Soul *soul)
{
	soul->drv->free(soul->drv);

	cmp_appr_anim_sprite_deinit(&soul->appr_walk_right);
	cmp_appr_anim_sprite_deinit(&soul->appr_walk_left);
	cmp_appr_static_sprite_deinit(&soul->appr_stand_right);
	cmp_appr_static_sprite_deinit(&soul->appr_stand_left);
	cmp_appr_static_sprite_deinit(&soul->appr_caught);
	cmp_appr_proxy_deinit(&soul->appr);

	cmp_ai_soul_deinit(&soul->ai);
	cmp_pain_deinit(&soul->pain);
	cmp_ori_deinit(&soul->ori);
}

void soul_tick(struct Soul *soul, struct CmpAiTacticalStatus *ts, double dt)
{
    struct Vel vel;

	soul->drv->update(soul->drv, dt);

	soul->appr.base.update(CMP_APPR(&soul->appr), dt);
    soul->ai.base.update(CMP_AI(&soul->ai), &soul->ori, soul->drv, ts, dt);

    vel = soul->drv->vel(soul->drv);

    if (vel.vx > 0) {
		cmp_appr_proxy_set_child(CMP_APPR(&soul->appr), SOUL_APPR_WALK_RIGHT);
    } else if (vel.vx < 0) {
		cmp_appr_proxy_set_child(CMP_APPR(&soul->appr), SOUL_APPR_WALK_LEFT);
    }

	cmp_think(CMP_AI(&soul->ai));
    cmp_drive(&soul->ori, soul->drv, dt);
}

void soul_draw(struct Soul *soul)
{
    struct WorldPos zero_wp = { 0.0, 0.0 };
    struct ScreenPos zero_sp = pos_world_to_screen(zero_wp);
	cmp_draw(&soul->ori, CMP_APPR(&soul->appr), -zero_sp.x, -zero_sp.y);
}

