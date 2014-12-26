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

	arrow->drv.base.update(CMP_DRV(&arrow->drv), dt);
	cmp_drive(&arrow->ori, CMP_DRV(&arrow->drv), dt);

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

static void sc_tick_hunter_input(struct Hunter *hunter, double dt)
{
	double rot_speed = 3.1415 / 2.0;
	int rot = sys_keys[ALLEGRO_KEY_UP] - sys_keys[ALLEGRO_KEY_DOWN];
	int hunter_appr = cmp_appr_proxy_get_child(CMP_APPR(&hunter->appr));

	bool right_facing =
		hunter_appr == HUNTER_APPR_WALK_RIGHT ||
		hunter_appr == HUNTER_APPR_STAND_RIGHT;

	bool left_facing =
		hunter_appr == HUNTER_APPR_WALK_LEFT ||
		hunter_appr == HUNTER_APPR_STAND_LEFT;

	if (right_facing) {
		hunter->aim_angle -= rot * rot_speed * dt;
	}
	else {
		hunter->aim_angle += rot * rot_speed * dt;
	}

	hunter->inx = sys_keys[ALLEGRO_KEY_RIGHT] - sys_keys[ALLEGRO_KEY_LEFT];
	hunter->jump_req = sys_keys[ALLEGRO_KEY_Z];

	if (hunter->standing) {
		if (hunter->inx == 1) {
			if (left_facing) {
				hunter->aim_angle = 3.1415 - hunter->aim_angle;
			}
			cmp_appr_proxy_set_child(CMP_APPR(&hunter->appr), HUNTER_APPR_WALK_RIGHT);

		}
		else if (hunter->inx == -1) {
			if (right_facing) {
				hunter->aim_angle = 3.1415 - hunter->aim_angle;
			}
			cmp_appr_proxy_set_child(CMP_APPR(&hunter->appr), HUNTER_APPR_WALK_LEFT);

		}
		else {
			if (left_facing) {
				cmp_appr_proxy_set_child(CMP_APPR(&hunter->appr), HUNTER_APPR_STAND_LEFT);
			}
			else {
				cmp_appr_proxy_set_child(CMP_APPR(&hunter->appr), HUNTER_APPR_STAND_RIGHT);
			}
		}
	}
}

void sc_tick_hunter(struct Hunter *hunter, double dt)
{
	hunter->drv.base.update(CMP_DRV(&hunter->drv), dt);
	hunter->appr.base.update(CMP_APPR(&hunter->appr), dt);
	cmp_drive(&hunter->ori, CMP_DRV(&hunter->drv), dt);
	sc_tick_hunter_input(hunter, dt);
}

void sc_tick_souls(struct SoulArray *souls, struct CmpAiTacticalStatus *ts, double dt)
{
	int i;
	for (i = 0; i < souls->size; ++i) {

		struct Vel vel;
		struct Soul *soul = souls->data + i;

		soul->drv.base.update(CMP_DRV(&soul->drv), dt);
		soul->appr.base.update(CMP_APPR(&soul->appr), dt);
		soul->ai.base.update(CMP_AI(&soul->ai), ts, dt);

		/* TODO: Cast the driver once in the begining of the function. */
		vel = soul->drv.base.vel(CMP_DRV(&soul->drv));

		if (vel.vx > 0) {
			cmp_appr_proxy_set_child(CMP_APPR(&soul->appr), SOUL_APPR_WALK_RIGHT);
		}
		else if (vel.vx < 0) {
			cmp_appr_proxy_set_child(CMP_APPR(&soul->appr), SOUL_APPR_WALK_LEFT);
		}

		cmp_think(CMP_AI(&soul->ai));
		cmp_drive(&soul->ori, CMP_DRV(&soul->drv), dt);
	}
}
