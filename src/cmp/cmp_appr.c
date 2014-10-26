/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <stdlib.h>

#include <allegro5/allegro.h>

#include "diagnostics.h"
#include "resources.h"
#include "cmp_appr.h"

struct CmpApprAnimSpriteCommon *cmp_appr_create_anim_sprite_common(
        void **frames, int frames_count,
        int *frame_indices, double *frame_times, int frame_defs_count,
        int frame_w)
{
    struct CmpApprAnimSpriteCommon *result = malloc(sizeof(*result));
    if (!result) {
        DIAG_ERROR("Allocation failure.");
        exit(1);
    }

    result->frames = frames;
    result->frames_count = frames_count;
    result->frame_indices = frame_indices;
    result->frame_times = frame_times;
    result->frame_defs_count = frame_defs_count;
    result->frame_w = frame_w;

    return result;
}

void cmp_appr_free_anim_sprite_common(struct CmpApprAnimSpriteCommon* common)
{
    res_dispose_frame_sheet(common->frames, common->frames_count);
    free(common->frame_indices);
    free(common->frame_times);
    free(common);
}

struct CmpAppr *cmp_appr_create_static_sprite(void *sprite)
{
    struct CmpAppr *result = malloc(sizeof(*result));
    if (!result) {
        DIAG_ERROR("Allocation failure.");
        exit(1);
    }

    result->type = CMP_APPR_STATIC_SPRITE;
    result->body.static_sprite = sprite;

    return result;
}

static void cmp_appr_update_anim_sprite(
        struct CmpApprAnimSprite *as,
        double dt)
{
    as->time_to_switch -= dt;

    if(!as->done && (as->time_to_switch <= 0.0)) {

        if(as->current_def == as->common->frame_defs_count - 1) {
            if(as->rep_count > 0) {
                --as->rep_count;
            }
            if(as->rep_count == 0) {
                as->done = true;
                return;
            }
        }

        double remain = -(as->time_to_switch);

        as->current_def = (as->current_def + 1) % as->common->frame_defs_count;
        as->time_to_switch = as->common->frame_times[as->current_def];
        as->time_to_switch -= remain;
    }
}

void *cmp_appr_bitmap_anim_sprite(struct CmpApprAnimSprite *as)
{
    return as->common->frames[as->common->frame_indices[as->current_def]];
}

struct CmpAppr *cmp_appr_create_anim_sprite(
        struct CmpApprAnimSpriteCommon *common, int init_def, int rep_count)
{
    struct CmpAppr *result = malloc(sizeof(*result));
    if (!result) {
        DIAG_ERROR("Allocation failure.");
        exit(1);
    }

    result->type = CMP_APPR_ANIM_SPRITE;

    result->body.anim_sprite.common = common;
    result->body.anim_sprite.current_def = init_def;
    result->body.anim_sprite.rep_count = rep_count;
    result->body.anim_sprite.time_to_switch = common->frame_times[init_def];
    result->body.anim_sprite.done = false;

    return result;
}

void cmp_appr_free(struct CmpAppr *cmp_appr)
{
    switch(cmp_appr->type) {
    case CMP_APPR_STATIC_SPRITE:
        break;
    case CMP_APPR_ANIM_SPRITE:
        break;
    default:
        DIAG_ERROR("Unhandled apprarance component type.");
        exit(1);
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
        DIAG_ERROR("Unhandled apprarance component type.");
        exit(1);
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
        DIAG_ERROR("Unhandled apprarance component type.");
        exit(1);
    }
}

