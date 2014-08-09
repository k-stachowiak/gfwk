/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <stdlib.h>

#include "cmp_appr.h"
#include "cmp_ori.h"
#include "cmp_drv.h"
#include "cmp_operations.h"
#include "sc_soul.h"

#include "resources.h"

static struct CmpAppr *soul_init_walk_appr(struct Soul *soul, void *walk_sheet)
{
    int i;

    void **frames;
    int frames_count;

    int *frame_indices;
    double *frame_times;
    int frame_defs_count;

    int frame_w = 74;
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

void soul_init(
        struct Soul *soul, double x, double y,
        void *stand_right_bitmap, void *stand_left_bitmap,
        void *walk_right_sheet, void *walk_left_sheet)
{
    soul->appr_walk_right = soul_init_walk_appr(soul, walk_right_sheet);
    soul->appr_walk_left = soul_init_walk_appr(soul, walk_left_sheet);

    soul->appr_stand_right =
        cmp_appr_create_static_sprite(stand_right_bitmap);

    soul->appr_stand_left =
        cmp_appr_create_static_sprite(stand_left_bitmap);

    soul->appr = soul->appr_walk_right;
    soul->ori = cmp_ori_create(x, y, 0.0);
    soul->drv = cmp_drv_create_patrol(0.0, 100.0, 0.0, 100.0, &(soul->turn_flag));

    soul->box_w = 30.0;
    soul->box_h = 60.0;
}

void soul_deinit(struct Soul *soul)
{
    cmp_drv_free(soul->drv);
    cmp_ori_free(soul->ori);
    cmp_appr_free(soul->appr_stand_right);
    cmp_appr_free(soul->appr_stand_left);
    cmp_appr_free(soul->appr_walk_right);
    cmp_appr_free(soul->appr_walk_left);
}

void soul_tick(struct Soul *soul, double dt)
{
    cmp_appr_update(soul->appr, dt);
    cmp_drv_update(soul->drv, dt);

    if (soul->turn_flag) {
        if (soul->appr == soul->appr_walk_right) {
            soul->appr = soul->appr_walk_left;
        } else if (soul->appr == soul->appr_walk_left) {
            soul->appr = soul->appr_walk_right;
        }
    }

    cmp_drive(soul->ori, soul->drv, dt);
}

void soul_draw(struct Soul *soul)
{
    struct WorldPos zero_wp = { 0.0, 0.0 };
    struct ScreenPos zero_sp = pos_world_to_screen(zero_wp);
    cmp_draw(soul->ori, soul->appr, -zero_sp.x, -zero_sp.y);
}
