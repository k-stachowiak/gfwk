/* Copyright (C) 2014 Krzysztof Stachowiak */

#ifndef CMP_APPR_H
#define CMP_APPR_H

#include <stdbool.h>

/* Common animation sub-component.
 * ===============================
 */

struct CmpApprAnimSpriteCommon {
    void **frames;
    int frames_count;
    int *frame_indices;
    double *frame_times;
    int frame_defs_count;
    int frame_w;
};

void cmp_appr_anim_sprite_common_free(struct CmpApprAnimSpriteCommon* common);
struct CmpApprAnimSpriteCommon *cmp_appr_anim_sprite_common_create(
        void **frames, int frames_count,
        int *frame_indices, double *frame_times, int frame_defs_count,
        int frame_w);

/* Appearance component.
 * =====================
 */

struct CmpAppr {
    void (*free)(struct CmpAppr*);
    void (*update)(struct CmpAppr*, double);
    void *(*bitmap)(struct CmpAppr*);
};

struct CmpAppr *cmp_appr_static_sprite_create(void *sprite);

struct CmpAppr *cmp_appr_anim_sprite_create(
        struct CmpApprAnimSpriteCommon *common,
        int init_def,
        int rep_count);

#endif
