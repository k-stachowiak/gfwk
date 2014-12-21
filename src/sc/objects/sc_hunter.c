/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <math.h>

#include "sc_data.h"

#include "sc_hunter.h"

void hunter_init(struct Hunter *hunter)
{
	cmp_appr_anim_sprite_init(&hunter->appr_walk_right, sc_hunter_walk_right_common, 2, -1);
	cmp_appr_anim_sprite_init(&hunter->appr_walk_left, sc_hunter_walk_left_common, 2, -1);
	cmp_appr_static_sprite_init(&hunter->appr_stand_right, sc_hunter_stand_right);
	cmp_appr_static_sprite_init(&hunter->appr_stand_left, sc_hunter_stand_left);

	hunter->appr_array[HUNTER_APPR_STAND_LEFT] = CMP_APPR(&hunter->appr_stand_left);
	hunter->appr_array[HUNTER_APPR_STAND_RIGHT] = CMP_APPR(&hunter->appr_stand_right);
	hunter->appr_array[HUNTER_APPR_WALK_LEFT] = CMP_APPR(&hunter->appr_walk_left);
	hunter->appr_array[HUNTER_APPR_WALK_RIGHT] = CMP_APPR(&hunter->appr_walk_right);

	cmp_drv_platform_init(&hunter->drv,	&hunter->inx, &hunter->jump_req, &hunter->standing);
	cmp_appr_proxy_init(&hunter->appr, hunter->appr_array, 4, HUNTER_APPR_STAND_RIGHT);
	cmp_ori_init(&hunter->ori, 0.0, 0.0, 0.0);

    hunter->aim_angle = 0.0;
    hunter->box_w = 30.0;
    hunter->box_h = 60.0;

    hunter->jump_req = false;
    hunter->standing = false;
}

void hunter_deinit(struct Hunter *hunter)
{
    cmp_drv_platform_deinit(&hunter->drv);
	cmp_appr_anim_sprite_deinit(&hunter->appr_walk_right);
	cmp_appr_anim_sprite_deinit(&hunter->appr_walk_left);
	cmp_appr_static_sprite_deinit(&hunter->appr_stand_right);
	cmp_appr_static_sprite_deinit(&hunter->appr_stand_left);
	cmp_appr_proxy_deinit(&hunter->appr);
	cmp_ori_deinit(&hunter->ori);
}

