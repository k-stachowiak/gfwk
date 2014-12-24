/* Copyright (C) 2014 Krzysztof Stachowiak */

#include "sc_data.h"

#include "sc_soul.h"

void soul_init(struct Soul *soul, long id, struct Graph *lgph, struct TilePos soul_tp)
{
    struct WorldPos wp = pos_tile_to_world_ground(soul_tp);

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

	cmp_drv_linear_init(&soul->drv_stand, false, 0.0, 0.0, 0.0);
	cmp_drv_waypoint_init(&soul->drv_walk, 150.0);

	soul->drv_array[SOUL_DRV_STAND] = CMP_DRV(&soul->drv_stand);
	soul->drv_array[SOUL_DRV_WALK] = CMP_DRV(&soul->drv_walk);

	soul->id = id;

	cmp_drv_proxy_init(&soul->drv, soul->drv_array, 2, SOUL_DRV_WALK);
	cmp_appr_proxy_init(&soul->appr, soul->appr_array, 5, SOUL_APPR_WALK_RIGHT);
	cmp_ori_init(&soul->ori, wp.x, wp.y, 0.0);
	cmp_pain_init(&soul->pain, PT_SOUL);
	cmp_shape_circle_init(&soul->shape, 25.0);

	cmp_ai_soul_init(&soul->ai,
		id,
		lgph,
		&soul->ori,
		CMP_DRV(&soul->drv),
		&soul->drv_walk,
		CMP_APPR(&soul->appr));

	soul->health = 100;
    soul->box_w = 30.0;
    soul->box_h = 60.0;
}

void soul_deinit(struct Soul *soul)
{
	cmp_appr_anim_sprite_deinit(&soul->appr_walk_right);
	cmp_appr_anim_sprite_deinit(&soul->appr_walk_left);
	cmp_appr_static_sprite_deinit(&soul->appr_stand_right);
	cmp_appr_static_sprite_deinit(&soul->appr_stand_left);
	cmp_appr_static_sprite_deinit(&soul->appr_caught);
	cmp_appr_proxy_deinit(&soul->appr);

	cmp_drv_linear_deinit(&soul->drv_stand);
	cmp_drv_waypoint_deinit(&soul->drv_walk);
	cmp_drv_proxy_deinit(&soul->drv);

	cmp_ai_soul_deinit(&soul->ai);
	cmp_pain_deinit(&soul->pain);
	cmp_ori_deinit(&soul->ori);
	cmp_shape_circle_deinit(&soul->shape);
}

