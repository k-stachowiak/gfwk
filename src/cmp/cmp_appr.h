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

/* Base class.
 * ===========
 */

struct CmpAppr {
    void (*free)(struct CmpAppr*);
    void (*update)(struct CmpAppr*, double);
    void *(*bitmap)(struct CmpAppr*);
};

#define CMP_APPR(T) (struct CmpAppr*)(T)

/* Static sprite.
 * ==============
 */

struct CmpApprStaticSprite {
	struct CmpAppr base;
	void *bitmap;
};

void cmp_appr_static_sprite_init(struct CmpApprStaticSprite *appr, void *sprite);
void cmp_appr_static_sprite_deinit(struct CmpApprStaticSprite *appr);
struct CmpAppr *cmp_appr_static_sprite_create(void *sprite);

/* Animated sprite.
 * ================
 */

struct CmpApprAnimSprite {
	struct CmpAppr base;
	struct CmpApprAnimSpriteCommon *common;
	int current_def;
	int rep_count;
	double time_to_switch;
	bool done;
};

void cmp_appr_anim_sprite_init(
		struct CmpApprAnimSprite *appr,
		struct CmpApprAnimSpriteCommon *common,
		int init_def,
		int rep_count);

void cmp_appr_anim_sprite_deinit(struct CmpApprAnimSprite *appr);

struct CmpAppr *cmp_appr_anim_sprite_create(
        struct CmpApprAnimSpriteCommon *common,
        int init_def,
        int rep_count);

/* Proxy.
 * ======
 */

struct CmpApprProxy {
	struct CmpAppr base;
	struct CmpAppr **children;
	int children_count;
	int current_child;
};

void cmp_appr_proxy_init(
		struct CmpApprProxy *appr,
		struct CmpAppr *children[],
		int children_count,
		int init_child);

void cmp_appr_proxy_deinit(struct CmpApprProxy *appr);

struct CmpAppr *cmp_appr_proxy_create(
		struct CmpAppr *children[],
		int children_count,
		int init_child);

int cmp_appr_proxy_get_child(struct CmpAppr *this);
void cmp_appr_proxy_set_child(struct CmpAppr *this, int child);

#endif
