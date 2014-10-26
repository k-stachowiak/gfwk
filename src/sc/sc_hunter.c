/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <math.h>

#include <allegro5/allegro_primitives.h>

#include "cmp_appr.h"
#include "cmp_ori.h"
#include "cmp_drv.h"
#include "cmp_operations.h"
#include "sc_hunter.h"
#include "resources.h"
#include "system.h"
#include "draw.h"

void hunter_init(struct Hunter *hunter)
{
    hunter->appr_walk_right = cmp_appr_create_anim_sprite(sc_hunter_walk_right_common, 2, -1);
    hunter->appr_walk_left = cmp_appr_create_anim_sprite(sc_hunter_walk_left_common, 2, -1);
    hunter->appr_stand_right = cmp_appr_create_static_sprite(sc_hunter_stand_right);
    hunter->appr_stand_left = cmp_appr_create_static_sprite(sc_hunter_stand_left);

    hunter->appr = hunter->appr_stand_right;
    hunter->ori = cmp_ori_create(0.0, 0.0, 0.0);
    hunter->drv = cmp_drv_create_platform(
            &hunter->inx, &hunter->jump_req, &hunter->standing);

    hunter->aim_angle = 0.0;
    hunter->box_w = 30.0;
    hunter->box_h = 60.0;

    hunter->jump_req = false;
    hunter->standing = false;
}

void hunter_deinit(struct Hunter *hunter)
{
    cmp_drv_free(hunter->drv);
    cmp_ori_free(hunter->ori);
    cmp_appr_free(hunter->appr_stand_right);
    cmp_appr_free(hunter->appr_stand_left);
    cmp_appr_free(hunter->appr_walk_right);
    cmp_appr_free(hunter->appr_walk_left);
}

static void hunter_tick_input(struct Hunter *hunter, double dt)
{
    double rot_speed = 3.1415 / 2.0;
    int rot = sys_keys[ALLEGRO_KEY_UP] - sys_keys[ALLEGRO_KEY_DOWN];

    if (hunter->appr == hunter->appr_walk_right ||
        hunter->appr == hunter->appr_stand_right) {
            hunter->aim_angle -= rot * rot_speed * dt;
    } else {
            hunter->aim_angle += rot * rot_speed * dt;
    }

    hunter->inx = sys_keys[ALLEGRO_KEY_RIGHT] - sys_keys[ALLEGRO_KEY_LEFT];
    hunter->jump_req = sys_keys[ALLEGRO_KEY_Z];

	if (hunter->standing) {
		if (hunter->inx == 1) {
			if (hunter->appr == hunter->appr_walk_left ||
				hunter->appr == hunter->appr_stand_left) {
					hunter->aim_angle = 3.1415 - hunter->aim_angle;
			}
			hunter->appr = hunter->appr_walk_right;

		} else if (hunter->inx == -1) {
			if (hunter->appr == hunter->appr_walk_right ||
				hunter->appr == hunter->appr_stand_right) {
					hunter->aim_angle = 3.1415 - hunter->aim_angle;
			}
			hunter->appr = hunter->appr_walk_left;
		}
		if (hunter->inx == 0) {
			if (hunter->appr == hunter->appr_walk_left) {
				hunter->appr = hunter->appr_stand_left;
			} else if (hunter->appr == hunter->appr_walk_right) {
				hunter->appr = hunter->appr_stand_right;
			}
		}
	}
}

void hunter_tick(struct Hunter *hunter, double dt)
{
    cmp_appr_update(hunter->appr, dt);
    cmp_drv_update(hunter->drv, dt);
    cmp_drive(hunter->ori, hunter->drv, dt);
    hunter_tick_input(hunter, dt);
}

void hunter_draw(struct Hunter *hunter)
{
    struct PosRot hunter_pr = cmp_ori_get(hunter->ori);
    struct WorldPos hunter_wp = { hunter_pr.x, hunter_pr.y - 15.0 };
    struct ScreenPos hunter_sp = pos_world_to_screen(hunter_wp);
    struct WorldPos zero_wp = { 0.0, 0.0 };
    struct ScreenPos zero_sp = pos_world_to_screen(zero_wp);

    cmp_draw(hunter->ori, hunter->appr, -zero_sp.x, -zero_sp.y);
    draw_bitmap(sc_bow_bitmap, hunter_sp.x, hunter_sp.y, hunter->aim_angle);
}

void arrow_init(struct Arrow *arrow, double x, double y, double angle)
{
    static double vel = 600.0;
    arrow->appr = cmp_appr_create_static_sprite(sc_arrow_bitmap);
    arrow->ori = cmp_ori_create(x, y, angle);
    arrow->drv = cmp_drv_create_ballistic(true, cos(angle) * vel, sin(angle) * vel);
}

void arrow_deinit(struct Arrow *arrow)
{
    cmp_appr_free(arrow->appr);
    cmp_ori_free(arrow->ori);
    cmp_drv_free(arrow->drv);
}

bool arrow_tick(struct Arrow *arrow, double dt)
{
    static int margin = 20;

    struct PosRot pr;
    struct WorldPos wp;
    struct ScreenPos sp;

    cmp_drv_update(arrow->drv, dt);
    cmp_drive(arrow->ori, arrow->drv, dt);

    pr = cmp_ori_get(arrow->ori);
    wp.x = pr.x;
    wp.y = pr.y;
    sp = pos_world_to_screen(wp);

    if (sp.x < margin || sp.x > (sc_screen_w - margin) ||
        sp.y < margin || sp.y > (sc_screen_h - margin)) {
        return false;
    } else {
        return true;
    }
}

