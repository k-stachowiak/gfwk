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

static struct CmpAppr *hunter_init_walk_appr(
        struct Hunter *hunter, void *walk_sheet)
{
    int i;

    void **frames;
    int frames_count;

    int *frame_indices;
    double *frame_times;
    int frame_defs_count;

    int frame_w = 106;
    double frame_time = 0.1;

    res_cut_frame_sheet(walk_sheet, frame_w, &frames, &frames_count);

    frame_defs_count = 8;
    frame_indices = malloc(frame_defs_count * sizeof(*frame_indices));
    frame_times = malloc(frame_defs_count * sizeof(*frame_times));

    frame_indices[0] = 0;
    frame_indices[1] = 1;
    frame_indices[2] = 2;
    frame_indices[3] = 3;
    frame_indices[4] = 4;
    frame_indices[5] = 3;
    frame_indices[6] = 2;
    frame_indices[7] = 1;
    for (i = 0; i < frame_defs_count; ++i) {
        frame_times[i] = frame_time;
    }

    return cmp_appr_create_anim_sprite(
        frames, frames_count,
        frame_indices, frame_times, frame_defs_count,
        frame_w, 2, -1);
}

void hunter_init(
        struct Hunter *hunter,
        void *stand_right_bitmap, void *stand_left_bitmap,
        void *walk_right_sheet, void *walk_left_sheet)
{
    hunter->appr_walk_right = hunter_init_walk_appr(hunter, walk_right_sheet);
    hunter->appr_walk_left = hunter_init_walk_appr(hunter, walk_left_sheet);

    hunter->appr_stand_right =
        cmp_appr_create_static_sprite(stand_right_bitmap);

    hunter->appr_stand_left =
        cmp_appr_create_static_sprite(stand_left_bitmap);

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

void arrow_init(
        struct Arrow *arrow,
        double x, double y, double angle,
        void *arrow_bitmap)
{
    double vel = 600.0;

    arrow->appr = cmp_appr_create_static_sprite(arrow_bitmap);
    arrow->ori = cmp_ori_create(x, y, angle);
    arrow->drv = cmp_drv_create_ballistic(
        true, cos(angle) * vel, sin(angle) * vel);
}

void arrow_deinit(struct Arrow *arrow)
{
    cmp_appr_free(arrow->appr);
    cmp_ori_free(arrow->ori);
    cmp_drv_free(arrow->drv);
}


