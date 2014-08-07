/* Copyright (C) 2014 Krzysztof Stachowiak */

#ifndef CMP_APPR_H
#define CMP_APPR_H

#include <stdbool.h>

enum CmpApprType {
    CMP_APPR_STATIC_SPRITE,
    CMP_APPR_ANIM_SPRITE
};

struct CmpApprAnimSprite {
    void **frames;
    int frames_count;
    int *frame_indices;
    double *frame_times;
    int frame_defs_count;

    int frame_w;
    int current_def;
    int rep_count;

    double time_to_switch;
    bool done;
};

struct CmpAppr {
    enum CmpApprType type;
    double csx, csy;
    union {
        void *static_sprite;
        struct CmpApprAnimSprite anim_sprite;
    } body;
};

struct CmpAppr *cmp_appr_create_static_sprite(
        double csx, double csy,
        void *sprite);

struct CmpAppr *cmp_appr_create_anim_sprite(
        double csx, double csy,
        void **frames, int frames_count,
        int *frame_indices, double *frame_times, int frame_defs_count,
        int frame_w, int init_def, int rep_count);

void cmp_appr_free(struct CmpAppr *cmp_appr);
void cmp_appr_update(struct CmpAppr *cmp_appr, double dt);
void *cmp_appr_bitmap(struct CmpAppr *cmp_appr);

#endif
