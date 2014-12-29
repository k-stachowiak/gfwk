/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <math.h>

#include "sc_data.h"

#include "sc_hunter.h"

void hunter_init(struct Hunter *hunter, long id)
{
	struct CmpAppr appr_array[4];

	cmp_appr_static_sprite_init(
		appr_array + HUNTER_APPR_STAND_RIGHT,
		sc_hunter_stand_right);

	cmp_appr_static_sprite_init(
		appr_array + HUNTER_APPR_STAND_LEFT,
		sc_hunter_stand_left);

	cmp_appr_anim_sprite_init(
		appr_array + HUNTER_APPR_WALK_RIGHT,
		sc_hunter_walk_right_common, 2, -1);

	cmp_appr_anim_sprite_init(
		appr_array + HUNTER_APPR_WALK_LEFT,
		sc_hunter_walk_left_common, 2, -1);

	hunter->has_soul = false;
	hunter->id = id;

	cmp_drv_platform_init(&hunter->drv,	&hunter->inx, &hunter->jump_req, &hunter->standing);
	cmp_appr_proxy_init(&hunter->appr, appr_array, 4, HUNTER_APPR_STAND_RIGHT);
	cmp_ori_init(&hunter->ori, 0.0, 0.0, 0.0);
	cmp_pain_init(&hunter->pain, PT_HUNTER);
	cmp_shape_circle_init(&hunter->shape, 25.0);

    hunter->aim_angle = 0.0;
    hunter->box_w = 30.0;
    hunter->box_h = 60.0;

    hunter->jump_req = false;
    hunter->standing = false;
}

void hunter_deinit(struct Hunter *hunter)
{
    cmp_drv_deinit(&hunter->drv);
	cmp_appr_deinit(&hunter->appr);
	cmp_ori_deinit(&hunter->ori);
	cmp_pain_deinit(&hunter->pain);
	cmp_shape_circle_deinit(&hunter->shape);
}

