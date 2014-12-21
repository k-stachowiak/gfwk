/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <stdlib.h>

#include <allegro5/allegro.h>

#include "diagnostics.h"
#include "memory.h"
#include "resources.h"
#include "cmp_appr.h"

/* Common animation sub-component.
 * ===============================
 */

void cmp_appr_anim_sprite_common_free(struct CmpApprAnimSpriteCommon* common)
{
    res_dispose_frame_sheet(common->frames, common->frames_count);
    free_or_die(common->frame_indices);
    free_or_die(common->frame_times);
    free_or_die(common);
}

struct CmpApprAnimSpriteCommon *cmp_appr_anim_sprite_common_create(
        void **frames, int frames_count,
        int *frame_indices, double *frame_times, int frame_defs_count,
        int frame_w)
{
    struct CmpApprAnimSpriteCommon *result = malloc_or_die(sizeof(*result));

    result->frames = frames;
    result->frames_count = frames_count;
    result->frame_indices = frame_indices;
    result->frame_times = frame_times;
    result->frame_defs_count = frame_defs_count;
    result->frame_w = frame_w;

    return result;
}

/* Appearance component.
 * =====================
 */

/* Common implementation.
 * ----------------------
 */

static void cmp_appr_common_free(struct CmpAppr *this)
{
    free_or_die(this);
}

/* Static sprite implementation.
 * -----------------------------
 */

static void cmp_appr_static_sprite_update(struct CmpAppr *this, double dt)
{
    (void)this;
    (void)dt;
}

static void *cmp_appr_static_sprite_bitmap(struct CmpAppr *this)
{
    struct CmpApprStaticSprite *derived = (struct CmpApprStaticSprite*)this;
    return derived->bitmap;
}

void cmp_appr_static_sprite_init(struct CmpApprStaticSprite *appr, void *sprite)
{
	appr->base.free = cmp_appr_common_free;
	appr->base.update = cmp_appr_static_sprite_update;
	appr->base.bitmap = cmp_appr_static_sprite_bitmap;
	appr->bitmap = sprite;
}

void cmp_appr_static_sprite_deinit(struct CmpApprStaticSprite *appr)
{
	(void)appr;
}

struct CmpAppr *cmp_appr_static_sprite_create(void *sprite)
{
    struct CmpApprStaticSprite *result = malloc_or_die(sizeof(*result));
	cmp_appr_static_sprite_init(result, sprite);
    return CMP_APPR(result);
}

/* Animated sprite implementation.
 * -------------------------------
 */

static void cmp_appr_anim_sprite_update(
        struct CmpAppr *this,
        double dt)
{
    struct CmpApprAnimSprite *derived = (struct CmpApprAnimSprite*)this;

    derived->time_to_switch -= dt;

    if(!derived->done && (derived->time_to_switch <= 0.0)) {

        if(derived->current_def == derived->common->frame_defs_count - 1) {
            if(derived->rep_count > 0) {
                --derived->rep_count;
            }
            if(derived->rep_count == 0) {
                derived->done = true;
                return;
            }
        }

        double remain = -(derived->time_to_switch);

        derived->current_def = (derived->current_def + 1) % derived->common->frame_defs_count;
        derived->time_to_switch = derived->common->frame_times[derived->current_def];
        derived->time_to_switch -= remain;
    }
}

static void *cmp_appr_anim_sprite_bitmap(struct CmpAppr *this)
{
    struct CmpApprAnimSprite *derived = (struct CmpApprAnimSprite*)this;
    return derived->common->frames[derived->common->frame_indices[derived->current_def]];
}

void cmp_appr_anim_sprite_init(
		struct CmpApprAnimSprite *appr,
		struct CmpApprAnimSpriteCommon *common,
		int init_def,
		int rep_count)
{
	appr->base.free = cmp_appr_common_free;
	appr->base.update = cmp_appr_anim_sprite_update;
	appr->base.bitmap = cmp_appr_anim_sprite_bitmap;

	appr->common = common;
	appr->current_def = init_def;
	appr->rep_count = rep_count;
	appr->time_to_switch = common->frame_times[init_def];
	appr->done = false;
}

void cmp_appr_anim_sprite_deinit(struct CmpApprAnimSprite *appr)
{
	(void)appr;
}

struct CmpAppr *cmp_appr_anim_sprite_create(
        struct CmpApprAnimSpriteCommon *common, int init_def, int rep_count)
{
    struct CmpApprAnimSprite *result = malloc_or_die(sizeof(*result));
	cmp_appr_anim_sprite_init(result, common, init_def, rep_count);
    return (struct CmpAppr*)result;
}

/* Proxy apperance implementation.
 * -------------------------------
 */

static void cmp_appr_proxy_update(struct CmpAppr *this, double dt)
{
	struct CmpApprProxy *derived = (struct CmpApprProxy*)this;
	struct CmpAppr *child = derived->children[derived->current_child];
	child->update(child, dt);
}

static void *cmp_appr_proxy_bitmap(struct CmpAppr *this)
{
	struct CmpApprProxy *derived = (struct CmpApprProxy*)this;
	struct CmpAppr *child = derived->children[derived->current_child];
	return child->bitmap(child);
}

void cmp_appr_proxy_init(
		struct CmpApprProxy *appr,
		struct CmpAppr *children[],
		int children_count,
		int init_child)
{
	appr->base.free = cmp_appr_common_free;
	appr->base.update = cmp_appr_proxy_update;
	appr->base.bitmap = cmp_appr_proxy_bitmap;

	appr->children = children;
	appr->children_count = children_count;
	appr->current_child = init_child;
}

void cmp_appr_proxy_deinit(struct CmpApprProxy *appr)
{
	(void)appr;
}

struct CmpAppr *cmp_appr_proxy_create(
		struct CmpAppr *children[],
		int children_count,
		int init_child)
{
	struct CmpApprProxy *result = malloc_or_die(sizeof(*result));
	cmp_appr_proxy_init(result, children, children_count, init_child);
	return (struct CmpAppr*)result;
}

int cmp_appr_proxy_get_child(struct CmpAppr *this)
{
	struct CmpApprProxy *derived = (struct CmpApprProxy*)this;
	return derived->current_child;
}

void cmp_appr_proxy_set_child(struct CmpAppr *this, int child)
{
	struct CmpApprProxy *derived = (struct CmpApprProxy*)this;
	derived->current_child = child;
}