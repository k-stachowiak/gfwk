/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <math.h>
#include <stdlib.h>

#include "diagnostics.h"

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

	hunter_set_state(hunter, HUNTER_STATE_STAND_RIGHT);

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

void hunter_set_state(struct Hunter *hunter, enum HunterState state)
{
	cmp_appr_deinit(&hunter->appr);
	switch (state) {
	case HUNTER_STATE_STAND_RIGHT:
		cmp_appr_static_sprite_init(&hunter->appr, sc_hunter_stand_right);
		break;
	case HUNTER_STATE_STAND_LEFT:
		cmp_appr_static_sprite_init(&hunter->appr, sc_hunter_stand_left);
		break;
	case HUNTER_STATE_WALK_RIGHT:
		cmp_appr_anim_sprite_init(&hunter->appr, sc_hunter_walk_right_common, 2, -1);
		break;
	case HUNTER_STATE_WALK_LEFT:
		cmp_appr_anim_sprite_init(&hunter->appr, sc_hunter_walk_left_common, 2, -1);
		break;
	}
}

enum HunterState hunter_get_state(struct Hunter *hunter)
{
	if (hunter->appr.type == CMP_APPR_STATIC_SPRITE &&
		hunter->appr.body.static_sprite.bitmap == sc_hunter_stand_right) {
		return HUNTER_STATE_STAND_RIGHT;

	} else if (
		hunter->appr.type == CMP_APPR_STATIC_SPRITE &&
		hunter->appr.body.static_sprite.bitmap == sc_hunter_stand_left) {
		return HUNTER_STATE_STAND_LEFT;

	} else if (
		hunter->appr.type == CMP_APPR_ANIMATED_SPRITE &&
		hunter->appr.body.animated_sprite.common == sc_hunter_walk_right_common) {
		return HUNTER_STATE_WALK_RIGHT;

	} else if (
		hunter->appr.type == CMP_APPR_ANIMATED_SPRITE &&
		hunter->appr.body.animated_sprite.common == sc_hunter_walk_left_common) {
		return HUNTER_STATE_WALK_LEFT;

	} else {
		DIAG_ERROR("Failed recognizing hunter state based on the appearance component.");
		exit(1);
	}
}