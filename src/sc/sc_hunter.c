/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <math.h>

#include <allegro5/allegro_primitives.h>

#include "cmp_operations.h"
#include "sc_hunter.h"
#include "resources.h"
#include "system.h"
#include "draw.h"

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

static void hunter_tick_input(struct Hunter *hunter, double dt)
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
    } else {
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

		} else if (hunter->inx == -1) {
			if (right_facing) {
				hunter->aim_angle = 3.1415 - hunter->aim_angle;
			}
			cmp_appr_proxy_set_child(CMP_APPR(&hunter->appr), HUNTER_APPR_WALK_LEFT);

		} else {
			if (left_facing) {
				cmp_appr_proxy_set_child(CMP_APPR(&hunter->appr), HUNTER_APPR_STAND_LEFT);
			} else {
				cmp_appr_proxy_set_child(CMP_APPR(&hunter->appr), HUNTER_APPR_STAND_RIGHT);
			}
		}
	}
}

void hunter_tick(struct Hunter *hunter, double dt)
{
	hunter->drv.base.update(CMP_DRV(&hunter->drv), dt);
	hunter->appr.base.update(CMP_APPR(&hunter->appr), dt);
	cmp_drive(&hunter->ori, CMP_DRV(&hunter->drv), dt);
    hunter_tick_input(hunter, dt);
}

void hunter_draw(struct Hunter *hunter)
{
    struct PosRot hunter_pr = cmp_ori_get(&hunter->ori);
    struct WorldPos hunter_wp = { hunter_pr.x, hunter_pr.y - 15.0 };
    struct ScreenPos hunter_sp = pos_world_to_screen(hunter_wp);
    struct WorldPos zero_wp = { 0.0, 0.0 };
    struct ScreenPos zero_sp = pos_world_to_screen(zero_wp);

	cmp_draw(&hunter->ori, CMP_APPR(&hunter->appr), -zero_sp.x, -zero_sp.y);
    draw_bitmap(sc_bow_bitmap, hunter_sp.x, hunter_sp.y, hunter->aim_angle);
}

