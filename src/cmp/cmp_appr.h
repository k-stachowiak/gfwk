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

struct CmpApprAnimSpriteCommon *cmp_appr_anim_sprite_common_create(
		void **frames, int frames_count,
		int *frame_indices, double *frame_times, int frame_defs_count,
		int frame_w);

void cmp_appr_anim_sprite_common_free(struct CmpApprAnimSpriteCommon* common);

/* Particular implementations.
 * ===========================
 */

struct CmpApprStaticSprite {
	void *bitmap;
};

struct CmpApprAnimSprite {
	struct CmpApprAnimSpriteCommon *common;
	int current_def;
	int rep_count;
	double time_to_switch;
	bool done;
};

/* Root struct.
 * ============
 */

enum CmpApprType {
	CMP_APPR_STATIC_SPRITE,
	CMP_APPR_ANIMATED_SPRITE
};

struct CmpAppr {
	union {
		struct CmpApprStaticSprite static_sprite;
		struct CmpApprAnimSprite animated_sprite;
	} body;
	enum CmpApprType type;
};

void cmp_appr_static_sprite_init(struct CmpAppr *appr, void *sprite);

void cmp_appr_anim_sprite_init(
		struct CmpAppr *appr,
		struct CmpApprAnimSpriteCommon *common,
		int init_def,
		int rep_count);

void cmp_appr_deinit(struct CmpAppr *appr);
void cmp_appr_update(struct CmpAppr* appr, double dt);
void *cmp_appr_bitmap(struct CmpAppr* appr);

#endif
