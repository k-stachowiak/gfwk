/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <math.h>

#include "sc_data.h"

#include "sc_hunter.h"

void hunter_init(struct Hunter *hunter, long id)
{
	hunter->has_soul = false;
	hunter->id = id;

	cmp_drv_platform_init(&hunter->drv, &hunter->inx, &hunter->jump_req, &hunter->standing);
	cmp_ori_init(&hunter->ori, 0.0, 0.0, 0.0);
	cmp_pain_init(&hunter->pain, PT_HUNTER);
	cmp_shape_circle_init(&hunter->shape, 25.0);

	hunter_set_appr_stand_right(&hunter->appr);

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

void hunter_set_appr_stand_right(struct CmpAppr *appr)
{
	cmp_appr_deinit(appr);
	cmp_appr_static_sprite_init(appr, sc_hunter_stand_right);
}

void hunter_set_appr_stand_left(struct CmpAppr *appr)
{
	cmp_appr_deinit(appr);
	cmp_appr_static_sprite_init(appr, sc_hunter_stand_left);
}

void hunter_set_appr_walk_right(struct CmpAppr *appr)
{
	cmp_appr_deinit(appr);
	cmp_appr_anim_sprite_init(appr, sc_hunter_walk_right_common, 2, -1);
}

void hunter_set_appr_walk_left(struct CmpAppr *appr)
{
	cmp_appr_deinit(appr);
	cmp_appr_anim_sprite_init(appr, sc_hunter_walk_left_common, 2, -1);
}