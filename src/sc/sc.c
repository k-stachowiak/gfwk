/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <stdbool.h>
#include <stdlib.h>

#include "memory.h"
#include "array.h"
#include "diagnostics.h"

#include "system.h"
#include "resources.h"
#include "database.h"

#include "menu.h"

#include "sc_data.h"
#include "sc_level.h"
#include "sc_graph.h"
#include "sc_hunter.h"
#include "sc_soul.h"
#include "sc_arrow.h"

#include "sc_arms.h"
#include "sc_draw.h"
#include "sc_tick.h"
#include "sc_platform.h"
#include "sc_pain.h"

/* Local state. */
static bool sc_alive;
static struct SysClient *sc_next;

struct {
	long *data;
	int size, cap;
} arrows_to_stick = { NULL, 0, 0 };

struct {
	long *data;
	int size, cap;
} souls_to_collect = { NULL, 0, 0 };

/* Resources management logic. */
static void sc_init_resources_basic(void)
{
    sc_debug_font = res_load_font("data/prstartk.ttf", 12);
    sc_tile = res_load_bitmap("data/brick_tile.png");
    sc_soulbooth = res_load_bitmap("data/soul_booth.png");
    sc_hunter_stand_right = res_load_bitmap("data/hunter_stand_right.png");
    sc_hunter_stand_left = res_load_bitmap("data/hunter_stand_left.png");
    sc_hunter_walk_right = res_load_bitmap("data/hunter_walk_right_w106.png");
    sc_hunter_walk_left = res_load_bitmap("data/hunter_walk_left_w106.png");
    sc_bow_bitmap = res_load_bitmap("data/bow.png");
    sc_arrow_bitmap = res_load_bitmap("data/arrow.png");
    sc_soul_stand_right = res_load_bitmap("data/soul_stand_right.png");
    sc_soul_stand_left = res_load_bitmap("data/soul_stand_left.png");
    sc_soul_walk_right = res_load_bitmap("data/soul_walk_right_w74.png");
    sc_soul_walk_left = res_load_bitmap("data/soul_walk_left_w74.png");
    sc_soul_caught = res_load_bitmap("data/soul_caught.png");
}

static void sc_deinit_resources_basic(void)
{
    res_dispose_font(sc_debug_font);
    res_dispose_bitmap(sc_tile);
    res_dispose_bitmap(sc_soulbooth);
    res_dispose_bitmap(sc_hunter_stand_right);
    res_dispose_bitmap(sc_hunter_stand_left);
    res_dispose_bitmap(sc_hunter_walk_right);
    res_dispose_bitmap(sc_hunter_walk_left);
    res_dispose_bitmap(sc_bow_bitmap);
    res_dispose_bitmap(sc_arrow_bitmap);
    res_dispose_bitmap(sc_soul_stand_right);
    res_dispose_bitmap(sc_soul_stand_left);
    res_dispose_bitmap(sc_soul_walk_right);
    res_dispose_bitmap(sc_soul_walk_left);
    res_dispose_bitmap(sc_soul_caught);
}

static struct CmpApprAnimSpriteCommon *sc_init_hunter_anim_common(void *walk_sheet)
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
    frame_indices = malloc_or_die(frame_defs_count * sizeof(*frame_indices));
    frame_times = malloc_or_die(frame_defs_count * sizeof(*frame_times));

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

    return cmp_appr_anim_sprite_common_create(
        frames, frames_count,
        frame_indices, frame_times, frame_defs_count,
        frame_w);
}

static struct CmpApprAnimSpriteCommon *sc_init_soul_anim_common(void *walk_sheet)
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
    frame_indices = malloc_or_die(frame_defs_count * sizeof(*frame_indices));
    frame_times = malloc_or_die(frame_defs_count * sizeof(*frame_times));

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

    return cmp_appr_anim_sprite_common_create(
        frames, frames_count,
        frame_indices, frame_times, frame_defs_count,
        frame_w);
}

static void sc_init_resources_complex(void)
{
    sc_hunter_walk_right_common = sc_init_hunter_anim_common(sc_hunter_walk_right);
    sc_hunter_walk_left_common = sc_init_hunter_anim_common(sc_hunter_walk_left);
    sc_soul_walk_right_common = sc_init_soul_anim_common(sc_soul_walk_right);
    sc_soul_walk_left_common = sc_init_soul_anim_common(sc_soul_walk_left);
}

static void sc_deinit_resources_complex(void)
{
    cmp_appr_anim_sprite_common_free(sc_hunter_walk_right_common);
    cmp_appr_anim_sprite_common_free(sc_hunter_walk_left_common);
    cmp_appr_anim_sprite_common_free(sc_soul_walk_right_common);
    cmp_appr_anim_sprite_common_free(sc_soul_walk_left_common);
}

/* Client API.
 * ===========
 */

static void sc_arrow_pain_callback(PainType pt_x, long id_x, PainType pt_y, void *data)
{
	ARRAY_APPEND(arrows_to_stick, id_x);
}

static void sc_arrows_pain_stick(void)
{
	int i, j;
	for (i = 0; i < arrows_to_stick.size; ++i) {
		long id = arrows_to_stick.data[i];

		for (j = 0; j < arrows.size; ++j) {
			struct Arrow *arrow = arrows.data + j;

			if (arrow->id != id) {
				continue;
			}

			arrow->timer = 1.0;
			ARRAY_APPEND(arrows_stuck, *arrow);
			ARRAY_REMOVE(arrows, j);
			break;
		}
	}
}

static void sc_soul_pain_callback(PainType pt_x, long id_x, PainType pt_y, void *data)
{
	struct Soul *soul = sc_soul_find_id(id_x);

	if (!soul) {
		DIAG_ERROR("Soul of id %ld not found.", id_x);
		exit(1);
	}

	if (soul->ai.body.soul.state != CMP_AI_SOUL_STATE_KO) {
		return;
	}

	if (pt_y != PT_HUNTER) {
		return;
	}

	ARRAY_APPEND(souls_to_collect, soul->id);
}

static void sc_souls_collect(void)
{
	int i;
	for (i = 0; i < souls_to_collect.size; ++i) {
		long soul_id = souls_to_collect.data[i];
		struct Soul *soul = sc_soul_find_id(soul_id);
		if (!soul) {
			DIAG_ERROR("Soul of id %ld not found.", soul_id);
			exit(1);
		}
	}
}

static void sc_init(void)
{
	struct Soul soul_memory_template = { 0 };

    sc_alive = true;
    sc_next = NULL;
	sc_entity_id = 0;

    sc_screen_w = db_integer("screen_w");
    sc_screen_h = db_integer("screen_h");
    sc_tile_w = 64;

    sc_init_resources_basic();
    sc_init_resources_complex();

	sc_pain_callback_type_register(PT_ARROW, NULL, sc_arrow_pain_callback);
	sc_pain_callback_type_register(PT_SOUL, NULL, sc_soul_pain_callback);

    lvl_load(&lvl, "data/map");
    lgph = lvl_init_graph(&lvl);

    hunter_init(&hunter, ++sc_entity_id);

	ARRAY_FREE(arrows);
	ARRAY_FREE(arrows_stuck);
	ARRAY_FREE(souls);

	ARRAY_APPEND(souls, soul_memory_template);
	ARRAY_APPEND(souls, soul_memory_template);
	ARRAY_APPEND(souls, soul_memory_template);

	soul_init(souls.data + 0, ++sc_entity_id, &lgph, lgph.nodes[1]);
	soul_init(souls.data + 1, ++sc_entity_id, &lgph, lgph.nodes[5]);
	soul_init(souls.data + 2, ++sc_entity_id, &lgph, lgph.nodes[10]);

    sc_cam_shift.x = 0.0;
    sc_cam_shift.y = 0.0;
}

static void sc_deinit(void)
{
	int i;

	lgph_deinit(&lgph);
	lvl_unload(&lvl);
	hunter_deinit(&hunter);

	for (i = 0; i < souls.size; ++i) {
		soul_deinit(souls.data + i);
	}

    sc_deinit_resources_complex();
    sc_deinit_resources_basic();
}

static void sc_tick(double dt)
{
	struct CmpAiTacticalStatus ts = {
		cmp_ori_get(&hunter.ori)
	};

	ARRAY_CLEAR(arrows_to_stick);
	ARRAY_CLEAR(souls_to_collect);

    sc_tick_camera(dt);
	sc_tick_arrows_regular(&arrows, dt);
	sc_tick_arrows_stuck(&arrows_stuck, dt);
	sc_tick_souls(&souls, &ts, dt);
	sc_tick_hunter(&hunter, dt);

	sc_platform_collide(&hunter, &lvl);

	sc_pain_tick(&arrows, &souls, &hunter);

	sc_arrows_pain_stick();
	sc_souls_collect();
}

static void sc_draw(double weight)
{
    al_clear_to_color(al_map_rgb_f(0.0, 0.0, 0.0));

	sc_draw_level(&lvl);
	sc_draw_hunter(&hunter);
	sc_draw_souls(&souls);
	sc_draw_arrows(&arrows);
    sc_draw_arrows(&arrows_stuck);

    if (!sys_keys[ALLEGRO_KEY_F1]) {
		sc_platform_draw_debug();
		sc_pain_draw_debug();
		sc_draw_graph_dbg(&lgph);
        sc_draw_souls_dbg(&souls);
    }

    al_flip_display();
}

static void sc_key(int key, bool down)
{
    if (down && key == ALLEGRO_KEY_ESCAPE) {
        sc_next = menu_get_client();
        sc_alive = false;
    }

    if (down && key == ALLEGRO_KEY_X) {
        sc_shoot_arrow();
    }
}

static struct SysClient sc_client = {
    &sc_alive,
    &sc_next,
    &sc_init,
    &sc_deinit,
    &sc_tick,
    &sc_draw,
    &sc_key
};

struct SysClient *sc_get_client(void)
{
    return &sc_client;
}

