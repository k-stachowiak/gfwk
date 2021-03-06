/* Copyright (C) 2014 Krzysztof Stachowiak */

#include "array.h"
#include "system.h"

#include "cmp_ori.h"
#include "cmp_drv.h"
#include "cmp_operations.h"

#include "sc_data.h"

#include "sc_hunter.h"
#include "sc_arrow.h"
#include "sc_soul.h"

void sc_tick_camera(double dt)
{
	struct PosRot hunter_pr = cmp_ori_get(&hunter.ori);
	sc_cam_shift.x = hunter_pr.x;
	sc_cam_shift.y = hunter_pr.y;
}

static bool arrow_tick(struct Arrow *arrow, double dt)
{
	static int margin = 20;

	struct PosRot pr;
	struct WorldPos wp;
	struct ScreenPos sp;

	cmp_drv_update(&arrow->drv, dt);
	cmp_drive(&arrow->ori, &arrow->drv, dt);

	pr = cmp_ori_get(&arrow->ori);
	wp.x = pr.x;
	wp.y = pr.y;
	sp = pos_world_to_screen(wp);

	if (sp.x < margin || sp.x >(sc_screen_w - margin) ||
		sp.y < margin || sp.y >(sc_screen_h - margin)) {
		return false;
	}
	else {
		return true;
	}
}

void sc_tick_arrows_regular(struct ArrowArray *array, double dt)
{
	int i;
	for (i = 0; i < array->size; ++i) {
		struct Arrow *arrow = array->data + i;
		if (!arrow_tick(arrow, dt)) {
			arrow_deinit(arrow);
			ARRAY_REMOVE(*array, i);
			--i;
		}
	}
}

void sc_tick_arrows_stuck(struct ArrowArray *array, double dt)
{
	int i;
	for (i = 0; i < array->size; ++i) {
		struct Arrow *arrow = array->data + i;
		if ((arrow->timer -= dt) <= 0) {
			arrow_deinit(arrow);
			ARRAY_REMOVE(*array, i);
			--i;
		}
	}
}

static void sc_tick_hunter_analyze_state(
		struct Hunter *hunter,
		bool *stand, bool *right)
{
	switch (hunter_get_state(&hunter->appr)) {
	case HUNTER_STATE_STAND_RIGHT:
		*stand = true;
		*right = true;
		break;

	case HUNTER_STATE_STAND_LEFT:
		*stand = true;
		*right = false;
		break;

	case HUNTER_STATE_WALK_RIGHT:
		*stand = false;
		*right = true;
		break;

	case HUNTER_STATE_WALK_LEFT:
		*stand = false;
		*right = false;
		break;
	}
}

static void sc_tick_hunter_input(struct Hunter *hunter, double dt)
{
	double rot_speed = 3.1415 / 2.0;
	int rot;
	bool stand;
	bool right;

	rot = sys_keys[ALLEGRO_KEY_UP] - sys_keys[ALLEGRO_KEY_DOWN];
	hunter->inx = sys_keys[ALLEGRO_KEY_RIGHT] - sys_keys[ALLEGRO_KEY_LEFT];
	hunter->jump_req = sys_keys[ALLEGRO_KEY_Z];

	sc_tick_hunter_analyze_state(hunter, &stand, &right);

	if (right) {
		hunter->aim_angle -= rot * rot_speed * dt;
	} else {
		hunter->aim_angle += rot * rot_speed * dt;
	}

	if (hunter->inx > 0) {
		/* Driven right. */
		if (stand || !right) {
			hunter_set_state(&hunter->appr, HUNTER_STATE_WALK_RIGHT);
		}
		if (!right) {
			hunter->aim_angle = 3.1415 - hunter->aim_angle;
		}

	} else if (hunter->inx < 0) {
		/* Driven left. */
		if (stand || right) {
			hunter_set_state(&hunter->appr, HUNTER_STATE_WALK_LEFT);
		}
		if (right) {
			hunter->aim_angle = 3.1415 - hunter->aim_angle;
		}

	} else {
		/* Not driven. */
		if (!stand) {
			if (right) {
				hunter_set_state(&hunter->appr, HUNTER_STATE_STAND_RIGHT);
			} else {
				hunter_set_state(&hunter->appr, HUNTER_STATE_STAND_LEFT);
			}
		}
	}
}

void sc_tick_hunter(struct Hunter *hunter, double dt)
{
	cmp_drv_update(&hunter->drv, dt);
	cmp_appr_update(&hunter->appr, dt);
	cmp_drive(&hunter->ori, &hunter->drv, dt);
	sc_tick_hunter_input(hunter, dt);
}

void sc_tick_souls(struct SoulArray *souls, struct CmpAiTacticalStatus *ts, double dt)
{
	int i;
	for (i = 0; i < souls->size; ++i) {
		struct Soul *soul = souls->data + i;
		cmp_appr_update(&soul->appr, dt);
		cmp_ai_update(&soul->ai, &soul->ori, &soul->drv, &soul->appr, ts, dt);
		cmp_drv_update(&soul->drv, dt);
		cmp_drive(&soul->ori, &soul->drv, dt);
	}
}
