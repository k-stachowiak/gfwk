/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <stdbool.h>

#include <allegro5/allegro_primitives.h>

#include "array.h"
#include "memory.h"
#include "sc.h"
#include "cmp_ori.h"
#include "cmp_drv.h"
#include "cmp_appr.h"
#include "cmp_ai.h"
#include "cmp_operations.h"
#include "sc_data.h"
#include "sc_hunter.h"
#include "sc_soul.h"
#include "sc_level.h"
#include "sc_graph.h"
#include "sc_platform.h"
#include "sc_pain.h"
#include "resources.h"
#include "database.h"
#include "menu.h"
#include "draw.h"

/* Local state. */
static bool sc_alive;
static struct SysClient *sc_next;

/* Entities. */
struct Level lvl;
struct Graph lgph;
struct Hunter hunter;
struct Soul soul;

struct ArrowArray {
    struct Arrow *data;
    int size, cap;
} arrows, arrows_stuck;

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

/* Common logic. */
static void sc_shoot_arrow(void)
{
    struct Arrow arrow;
    struct PosRot hunter_pr = cmp_ori_get(hunter.ori);
    struct WorldPos hunter_wp = { hunter_pr.x, hunter_pr.y };
    arrow_init(&arrow, hunter_wp.x, hunter_wp.y - 15.0, hunter.aim_angle);
    ARRAY_APPEND(arrows, arrow);
}

static void sc_tick_camera(double dt)
{
    struct PosRot hunter_pr = cmp_ori_get(hunter.ori);
    sc_cam_shift.x = hunter_pr.x;
    sc_cam_shift.y = hunter_pr.y;
}

/* Client API.
 * ===========
 */

static void sc_init(void)
{
    sc_alive = true;
    sc_next = NULL;

    sc_screen_w = db_integer("screen_w");
    sc_screen_h = db_integer("screen_h");
    sc_tile_w = 64;

    sc_init_resources_basic();
    sc_init_resources_complex();

    lvl_load(&lvl, "data/map");
    lgph = lvl_init_graph(&lvl);

    hunter_init(&hunter);
    soul_init(&soul, &lgph, lgph.nodes[10]);

    arrows.data = NULL;
    arrows.size = 0;
    arrows.cap = 0;
    arrows_stuck.data = NULL;
    arrows_stuck.size = 0;
    arrows_stuck.cap = 0;

    sc_cam_shift.x = 0.0;
    sc_cam_shift.y = 0.0;
}

static void sc_deinit(void)
{
    lgph_deinit(&lgph);
    lvl_unload(&lvl);
    hunter_deinit(&hunter);
    soul_deinit(&soul);

    sc_deinit_resources_complex();
    sc_deinit_resources_basic();
}

/** Dumb objects' frame (e.g. projectiles). */
static void sc_tick_dumb(double dt)
{
    int i;

    hunter_tick(&hunter, dt);

    for (i = 0; i < arrows.size; ++i) {
        struct Arrow *arrow = arrows.data + i;
        if (!arrow_tick(arrow, dt)) {
            arrow_deinit(arrow);
            ARRAY_REMOVE(arrows, i);
            --i;
        }
    }

    for (i = 0; i < arrows_stuck.size; ++i) {
        struct Arrow *arrow = arrows_stuck.data + i;
        if ((arrow-> timer -= dt) <= 0) {
            arrow_deinit(arrow);
            ARRAY_REMOVE(arrows_stuck, i);
            --i;
        }
    }
}

/** Smart object's frame (wiht AI). */
static void sc_tick_smart(struct CmpAiTacticalStatus *ts, double dt)
{
    soul_tick(&soul, ts, dt);
}

static void sc_tick_pain_feedback(void)
{
    int i, j;
    for (i = 0; i < arrows.size; ++i) {
        
        struct Arrow *arrow = arrows.data + i;
        struct CmpPain *pain = arrow->pain;

        for (j = 0; j < pain->queue_size; ++j) {
            if (pain->queue[j] == PT_SOUL) {
                arrow->timer = 1.0;
                ARRAY_APPEND(arrows_stuck, *arrow);
                ARRAY_REMOVE(arrows, i);
                --i;
                break;
            }
        }
    }
}

static void sc_tick(double dt)
{
    struct CmpAiTacticalStatus ts;

    sc_tick_camera(dt);
    sc_tick_dumb(dt);
    ts.hunter_pos = cmp_ori_get(hunter.ori);
    sc_tick_smart(&ts, dt);

    platform_collide(&hunter, &lvl);            /* Update platformer engine. */
    pain_tick(arrows.data, arrows.size, &soul); /* Update pain engine. */
    sc_tick_pain_feedback();
}

static void sc_draw_debug_graph(void)
{
    int i;
    char buf[10];

    for (i = 0; i < lgph.nodes_count; ++i) {

        struct Adj *adj;
        struct TilePos atp = lgph.nodes[i];
		struct WorldPos awp = pos_tile_to_world_ground(atp);
        struct ScreenPos asp = pos_world_to_screen(awp);
        double x1 = asp.x;
        double y1 = asp.y;

        for (adj = lgph.adjacency[i]; adj->neighbor != -1; ++adj) {

            struct TilePos btp = lgph.nodes[adj->neighbor];
			struct WorldPos bwp = pos_tile_to_world_ground(btp);
            struct ScreenPos bsp = pos_world_to_screen(bwp);
            double x2 = bsp.x;
            double y2 = bsp.y;

            ALLEGRO_COLOR color;
            switch (adj->type) {
            case ADJ_WALK:
                color = al_map_rgb_f(0.5f, 1.0f, 0.5f);
                break;
            case ADJ_JUMP:
                color = al_map_rgb_f(0.5f, 0.5f, 1.0f);
                break;
            }

            al_draw_line(x1, y1, x2, y2, color, 1.0);
            al_draw_filled_circle(x1 - 4.0, y1, 5.0, color);
            al_draw_circle(x2 + 4.0, y2, 5.0, color, 1.0);

            sprintf(buf, "%d", i);
            draw_text(sc_debug_font, x1 + 2, y1 - 18, 0, 0, 0, 1, true, buf);
            draw_text(sc_debug_font, x1, y1 - 20, 1, 1, 1, 1, true, buf);

            sprintf(buf, "%d", adj->neighbor);
            draw_text(sc_debug_font, x2 + 2, y2 - 18, 0, 0, 0, 1, true, buf);
            draw_text(sc_debug_font, x2, y2 - 20, 1, 1, 1, 1, true, buf);

        }
    }
}

static void sc_draw_debug_soul(struct Soul *soul)
{
    int i;
    double *points;
    int points_count;

    ALLEGRO_COLOR color = al_map_rgb_f(1.0f, 0.7f, 0.4f);

	struct PosRot soul_pr = cmp_ori_get(soul->ori);
	struct WorldPos soul_wp = { soul_pr.x, soul_pr.y };
	struct ScreenPos soul_sp = pos_world_to_screen(soul_wp);

	al_draw_circle(soul_sp.x, soul_sp.y, 4, color, 1);

    cmp_drv_waypoint_points(soul->drv, &points, &points_count);

    for (i = 0; i < (points_count - 1); ++i) {

        struct WorldPos
            wp1 = { points[2 * i + 0], points[2 * i + 1] },
            wp2 = { points[2 * i + 2], points[2 * i + 3] };

        struct ScreenPos
            sp1 = pos_world_to_screen(wp1),
            sp2 = pos_world_to_screen(wp2);

        double x1 = sp1.x;
        double y1 = sp1.y;
        double x2 = sp2.x;
        double y2 = sp2.y;

        al_draw_line(x1, y1, x2, y2, color, 2.0);
    }
}

static void sc_draw_arrow_array(struct ArrowArray *aa)
{
    int i;
    struct WorldPos zero_wp = { 0.0, 0.0 };
    struct ScreenPos zero_sp = pos_world_to_screen(zero_wp);
    for (i = 0; i < aa->size; ++i) {
        cmp_draw(aa->data[i].ori, aa->data[i].appr, -zero_sp.x, -zero_sp.y);
    }
}

static void sc_draw(double weight)
{
    al_clear_to_color(al_map_rgb_f(0.0, 0.0, 0.0));
    lvl_draw(&lvl);
    hunter_draw(&hunter);
    soul_draw(&soul);
    sc_draw_arrow_array(&arrows);
    sc_draw_arrow_array(&arrows_stuck);

    if (!sys_keys[ALLEGRO_KEY_F1]) {
        platform_draw_debug();
        pain_draw_debug();
        sc_draw_debug_graph();
        sc_draw_debug_soul(&soul);
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

