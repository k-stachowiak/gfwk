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

void cmp_appr_anim_sprite_common_free(struct CmpApprAnimSpriteCommon* common)
{
    res_dispose_frame_sheet(common->frames, common->frames_count);
    free_or_die(common->frame_indices);
    free_or_die(common->frame_times);
    free_or_die(common);
}

/* Static sprite implementation.
 * =============================
 */

static void cmp_appr_static_sprite_deinit(struct CmpApprStaticSprite *appr)
{
	(void)appr;
}

static void cmp_appr_static_sprite_update(struct CmpApprStaticSprite *appr, double dt)
{
	(void)appr;
	(void)dt;
}

static void *cmp_appr_static_sprite_bitmap(struct CmpApprStaticSprite *appr)
{
	return appr->bitmap;
}

/* Animated sprite implementation.
 * ===============================
 */

static void cmp_appr_animated_sprite_deinit(struct CmpApprAnimSprite *appr)
{
	(void)appr;
}

static void cmp_appr_animated_sprite_update(
		struct CmpApprAnimSprite *appr,
		double dt)
{
	appr->time_to_switch -= dt;

	if (!appr->done && (appr->time_to_switch <= 0.0)) {

		if (appr->current_def == appr->common->frame_defs_count - 1) {
			if (appr->rep_count > 0) {
				--appr->rep_count;
			}
			if (appr->rep_count == 0) {
				appr->done = true;
				return;
			}
		}

		double remain = -(appr->time_to_switch);

		appr->current_def = (appr->current_def + 1) % appr->common->frame_defs_count;
		appr->time_to_switch = appr->common->frame_times[appr->current_def];
		appr->time_to_switch -= remain;
	}
}

static void *cmp_appr_animated_sprite_bitmap(struct CmpApprAnimSprite *appr)
{
	return appr->common->frames[appr->common->frame_indices[appr->current_def]];
}

/* Proxy appearance implementation.
 * ================================
 */

static void cmp_appr_proxy_deinit(struct CmpApprProxy *appr)
{
	int i;
	for (i = 0; i < appr->children_count; ++i) {
		cmp_appr_deinit(appr->children + i);
	}
	free_or_die(appr->children);
}

static void cmp_appr_proxy_update(struct CmpApprProxy *appr, double dt)
{
	struct CmpAppr *child = appr->children + appr->current_child;
	cmp_appr_update(child, dt);
}

static void *cmp_appr_proxy_bitmap(struct CmpApprProxy *appr)
{
	struct CmpAppr *child = appr->children + appr->current_child;
	return cmp_appr_bitmap(child);
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

/* Public API.
 * ===========
 */

void cmp_appr_static_sprite_init(struct CmpAppr *appr, void *sprite)
{
	appr->type = CMP_APPR_STATIC_SPRITE;
	appr->body.static_sprite.bitmap = sprite;
}

void cmp_appr_anim_sprite_init(
		struct CmpAppr *appr,
		struct CmpApprAnimSpriteCommon *common,
		int init_def,
		int rep_count)
{
	appr->type = CMP_APPR_ANIMATED_SPRITE;
	appr->body.animated_sprite.common = common;
	appr->body.animated_sprite.current_def = init_def;
	appr->body.animated_sprite.rep_count = rep_count;
	appr->body.animated_sprite.time_to_switch = common->frame_times[init_def];
	appr->body.animated_sprite.done = false;
}

void cmp_appr_proxy_init(
		struct CmpAppr *appr,
		struct CmpAppr children[],
		int children_count,
		int init_child)
{
	appr->type = CMP_APPR_PROXY;
	appr->body.proxy.children_count = children_count;
	appr->body.proxy.current_child = init_child;

	memcpy(appr->body.proxy.children, children, sizeof(*children) * children_count);
}

void cmp_appr_deinit(struct CmpAppr *appr)
{
	switch (appr->type) {
	case CMP_APPR_STATIC_SPRITE:
		cmp_appr_static_sprite_deinit((struct CmpApprStaticSprite*)appr);
		break;
	case CMP_APPR_ANIMATED_SPRITE:
		cmp_appr_animated_sprite_deinit((struct CmpApprAnimSprite*)appr);
		break;
	case CMP_APPR_PROXY:
		cmp_appr_proxy_deinit((struct CmpApprProxy*)appr);
		break;
	}
}

void cmp_appr_update(struct CmpAppr *appr, double dt)
{
	switch (appr->type) {
	case CMP_APPR_STATIC_SPRITE:
		cmp_appr_static_sprite_update((struct CmpApprStaticSprite*)appr, dt);
		break;
	case CMP_APPR_ANIMATED_SPRITE:
		cmp_appr_animated_sprite_update((struct CmpApprAnimSprite*)appr, dt);
		break;
	case CMP_APPR_PROXY:
		cmp_appr_proxy_update((struct CmpApprProxy*)appr, dt);
		break;
	}
}

void *cmp_appr_bitmap(struct CmpAppr *appr)
{
	switch (appr->type) {
	case CMP_APPR_STATIC_SPRITE:
		return cmp_appr_static_sprite_bitmap((struct CmpApprStaticSprite*)appr);
	case CMP_APPR_ANIMATED_SPRITE:
		return cmp_appr_animated_sprite_bitmap((struct CmpApprAnimSprite*)appr);
	case CMP_APPR_PROXY:
		return cmp_appr_proxy_bitmap((struct CmpApprProxy*)appr);
	}
	DIAG_ERROR("Should never get here.");
	exit(1);
}