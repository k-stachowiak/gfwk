/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <stdlib.h>

#include "diagnostics.h"

#include "sc_data.h"

#include "sc_soul.h"

void soul_init(struct Soul *soul, long id, struct Graph *lgph, struct TilePos soul_tp)
{
	struct WorldPos wp = pos_tile_to_world_mid(soul_tp);

	soul->id = id;
	cmp_ori_init(&soul->ori, wp.x, wp.y, 0.0);
	cmp_drv_linear_init(&soul->drv, false, 0, 0, 0);
	cmp_appr_static_sprite_init(&soul->appr, sc_soul_stand_right);
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
	cmp_shape_deinit(&soul->shape);
}

void soul_start_walking(
		struct CmpDrv *drv,
		double *points, int points_count,
		CmpDrvCallback on_end, void *data)
{
	cmp_drv_waypoint_init(drv, 100.0);
	cmp_drv_waypoint_reset(drv, points, points_count);
	cmp_drv_waypoint_on_end(drv, on_end, data);
}

bool soul_walking(struct CmpAppr *appr)
{
	return appr->type == CMP_APPR_ANIMATED_SPRITE && (
		   appr->body.animated_sprite.common == sc_soul_walk_right_common ||
		   appr->body.animated_sprite.common == sc_soul_walk_left_common);
}

enum SoulDir soul_walking_dir(struct CmpAppr *appr)
{
	if (appr->type == CMP_APPR_ANIMATED_SPRITE &&
		appr->body.animated_sprite.common == sc_soul_walk_right_common) {
		return SOUL_DIR_RIGHT;
	}

	if (appr->type == CMP_APPR_ANIMATED_SPRITE &&
		appr->body.animated_sprite.common == sc_soul_walk_left_common) {
		return SOUL_DIR_LEFT;
	}

	DIAG_ERROR("Queried for a walking directory of a non-walking soul object.");
	exit(1);
}

void soul_update_walking(struct CmpAppr *appr, enum SoulDir dir)
{
	switch (dir) {
	case SOUL_DIR_RIGHT:
		cmp_appr_anim_sprite_init(appr, sc_soul_walk_right_common, 2, -1);
		break;

	case SOUL_DIR_LEFT:
		cmp_appr_anim_sprite_init(appr, sc_soul_walk_left_common, 2, -1);
		break;
	}	
}

void soul_knock_out(struct CmpAppr *appr, struct CmpDrv *drv)
{
	cmp_appr_static_sprite_init(appr, sc_soul_caught);
	cmp_drv_linear_init(drv, false, 0, 0, 0);
}