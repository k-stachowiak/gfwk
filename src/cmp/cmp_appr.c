/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <stdlib.h>

#include "diagnostics.h"
#include "resources.h"
#include "cmp_appr.h"

struct CmpAppr *cmp_appr_create_static_sprite(void *sprite)
{
    struct CmpAppr *result = malloc(sizeof(*result));
    if (!result) {
        DIAG_ERROR("Allocation failure.\n");
    }

    result->type = CMP_APPR_STATIC_SPRITE;
    result->body.static_sprite = sprite;

    return result;
}

static void cmp_appr_free_anim_sprite(struct CmpApprAnimSprite *anim_sprite)
{
    int i;
    for (i = 0; i < anim_sprite->frames_count; ++i) {
        res_dispose_bitmap(anim_sprite->frames[i]);
    }
    free(anim_sprite->frames);
    free(anim_sprite->frame_indices);
    free(anim_sprite->frame_times);
}

static void cmp_appr_update_anim_sprite(
        struct CmpApprAnimSprite *as,
        double dt)
{
    as->time_to_switch -= dt;

    if(!as->done && (as->time_to_switch <= 0.0)) {

        if(as->current_def == as->frame_defs_count - 1) {
            if(as->rep_count > 0) {
                --as->rep_count;
            }
            if(as->rep_count == 0) {
                as->done = true;
                return;
            }
        }

        double remain = -(as->time_to_switch);

        as->current_def = (as->current_def + 1) % as->frame_defs_count;
        as->time_to_switch = as->frame_times[as->current_def];
        as->time_to_switch -= remain;
    }
}

void *cmp_appr_bitmap_anim_sprite(struct CmpApprAnimSprite *as)
{
    return as->frames[as->frame_indices[as->current_def]];
}

struct CmpAppr *cmp_appr_create_anim_sprite(
        void **frames, int frames_count,
        int *frame_indices, double *frame_times, int frame_defs_count,
        int frame_w, int init_def, int rep_count)
{
    struct CmpAppr *result = malloc(sizeof(*result));
    if (!result) {
        DIAG_ERROR("Allocation failure.\n");
    }

    result->type = CMP_APPR_ANIM_SPRITE;

    result->body.anim_sprite.frames = frames;
    result->body.anim_sprite.frames_count = frames_count;
    result->body.anim_sprite.frame_indices = frame_indices;
    result->body.anim_sprite.frame_times = frame_times;
    result->body.anim_sprite.frame_defs_count = frame_defs_count;

    result->body.anim_sprite.frame_w = frame_w;
    result->body.anim_sprite.current_def = init_def;
    result->body.anim_sprite.rep_count = rep_count;

    result->body.anim_sprite.time_to_switch = frame_times[init_def];
    result->body.anim_sprite.done = false;

    return result;
}

void cmp_appr_free(struct CmpAppr *cmp_appr)
{
    switch(cmp_appr->type) {
    case CMP_APPR_STATIC_SPRITE:
        break;
    case CMP_APPR_ANIM_SPRITE:
        cmp_appr_free_anim_sprite(&cmp_appr->body.anim_sprite);
        break;
    default:
        DIAG_ERROR("Unhandled apprarance component type.\n");
    }
    free(cmp_appr);
}

void cmp_appr_update(struct CmpAppr *cmp_appr, double dt)
{
    switch(cmp_appr->type) {
    case CMP_APPR_STATIC_SPRITE:
        break;
    case CMP_APPR_ANIM_SPRITE:
        cmp_appr_update_anim_sprite(&cmp_appr->body.anim_sprite, dt);
        break;
    default:
        DIAG_ERROR("Unhandled apprarance component type.\n");
    }
}

void *cmp_appr_bitmap(struct CmpAppr *cmp_appr)
{
    switch(cmp_appr->type) {
    case CMP_APPR_STATIC_SPRITE:
        return cmp_appr->body.static_sprite;
    case CMP_APPR_ANIM_SPRITE:
        return cmp_appr_bitmap_anim_sprite(&cmp_appr->body.anim_sprite);
    default:
        DIAG_ERROR("Unhandled apprarance component type.\n");
    }
}

