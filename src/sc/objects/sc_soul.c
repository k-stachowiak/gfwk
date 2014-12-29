/* Copyright (C) 2014 Krzysztof Stachowiak */

#include "sc_data.h"

#include "sc_soul.h"

void soul_init(struct Soul *soul, long id, struct Graph *lgph, struct TilePos soul_tp)
{
    struct WorldPos wp = pos_tile_to_world_ground(soul_tp);
	struct CmpAppr appr_array[5];
	struct CmpDrv drv_array[2];

	cmp_appr_static_sprite_init(
		appr_array + SOUL_APPR_STAND_RIGHT,
		sc_soul_stand_right);

	cmp_appr_static_sprite_init(
		appr_array + SOUL_APPR_STAND_RIGHT,
		sc_soul_stand_left);

	cmp_appr_anim_sprite_init(
		appr_array + SOUL_APPR_WALK_RIGHT,
		sc_soul_walk_right_common, 2, -1);

	cmp_appr_anim_sprite_init(
		appr_array + SOUL_APPR_WALK_RIGHT,
		sc_soul_walk_left_common, 2, -1);

	cmp_appr_static_sprite_init(
		appr_array + SOUL_APPR_CAUGHT,
		sc_soul_caught);

	cmp_drv_linear_init(
		drv_array + SOUL_DRV_STAND,
		false, 0.0, 0.0, 0.0);

	cmp_drv_waypoint_init(
		drv_array + SOUL_DRV_WALK,
		150.0);

	soul->id = id;

	cmp_drv_proxy_init(&soul->drv, drv_array, 2, SOUL_DRV_WALK);
	cmp_appr_proxy_init(&soul->appr, appr_array, 5, SOUL_APPR_WALK_RIGHT);
	cmp_ori_init(&soul->ori, wp.x, wp.y, 0.0);
	cmp_pain_init(&soul->pain, PT_SOUL);
	cmp_shape_circle_init(&soul->shape, 25.0);

	cmp_ai_soul_init(&soul->ai, id, lgph);

	soul->health = 100;
    soul->box_w = 30.0;
    soul->box_h = 60.0;
}

void soul_deinit(struct Soul *soul)
{
	cmp_drv_deinit(&soul->drv);
	cmp_appr_deinit(&soul->appr);
	cmp_ai_deinit(&soul->ai);
	cmp_pain_deinit(&soul->pain);
	cmp_ori_deinit(&soul->ori);
	cmp_shape_circle_deinit(&soul->shape);
}

