/* Copyright (C) 2014 Krzysztof Stachowiak */

#include "sc_data.h"

#include "sc_soul.h"

void soul_init(struct Soul *soul, long id, struct Graph *lgph, struct TilePos soul_tp)
{
    struct WorldPos wp = pos_tile_to_world_ground(soul_tp);
	struct CmpDrv drv_array[2];

	soul->id = id;

	cmp_drv_proxy_init(&soul->drv, drv_array, 2, SOUL_DRV_WALK);
	cmp_ori_init(&soul->ori, wp.x, wp.y, 0.0);
	cmp_pain_init(&soul->pain, PT_SOUL);
	cmp_shape_circle_init(&soul->shape, 25.0);

	cmp_ai_soul_init(&soul->ai, id, lgph);

	soul_set_appr_stand_right(&soul->appr);

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

void soul_set_appr_stand_right(struct CmpAppr *appr)
{
	cmp_appr_deinit(appr);
	cmp_appr_static_sprite_init(appr, sc_soul_stand_right);
}

void soul_set_appr_stand_left(struct CmpAppr *appr)
{
	cmp_appr_deinit(appr);
	cmp_appr_static_sprite_init(appr, sc_soul_stand_left);
}

void soul_set_appr_walk_right(struct CmpAppr *appr)
{
	cmp_appr_deinit(appr);
	cmp_appr_anim_sprite_init(appr, sc_soul_walk_right_common, 2, -1);
}

void soul_set_appr_walk_left(struct CmpAppr *appr)
{
	cmp_appr_deinit(appr);
	cmp_appr_anim_sprite_init(appr, sc_soul_walk_left_common, 2, -1);
}

void soul_set_appr_caught(struct CmpAppr *appr)
{
	cmp_appr_deinit(appr);
	cmp_appr_static_sprite_init(appr, sc_soul_caught);
}
