/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <stdbool.h>

#include <allegro5/allegro_primitives.h>

#include "array.h"
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
#include "sc_collision.h"
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

struct {
    struct Arrow *data;
    int size, cap;
} arrows;

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
    frame_indices = malloc(frame_defs_count * sizeof(*frame_indices));
    frame_times = malloc(frame_defs_count * sizeof(*frame_times));

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
    frame_indices = malloc(frame_defs_count * sizeof(*frame_indices));
    frame_times = malloc(frame_defs_count * sizeof(*frame_times));

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

static void sc_update_screen_aabb(void)
{
    sc_screen_aabb.ax = sc_cam_shift.x - sc_screen_w / 2;
    sc_screen_aabb.ay = sc_cam_shift.y - sc_screen_h / 2;
    sc_screen_aabb.bx = sc_screen_aabb.ax + sc_screen_w;
    sc_screen_aabb.by = sc_screen_aabb.ay + sc_screen_h;
}

static void sc_tick_camera(double dt)
{
    struct PosRot hunter_pr = cmp_ori_get(hunter.ori);
    sc_cam_shift.x = hunter_pr.x;
    sc_cam_shift.y = hunter_pr.y;
    sc_update_screen_aabb();
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

    sc_cam_shift.x = 0.0;
    sc_cam_shift.y = 0.0;
    sc_update_screen_aabb();
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

static void sc_tick_dumb(double dt)
{
    int i;
    hunter_tick(&hunter, dt);
    for (i = 0; i < arrows.size; ++i) {
        if (!arrow_tick(arrows.data + i, dt)) {
            arrow_deinit(arrows.data + i);
            ARRAY_REMOVE(arrows, i);
            --i;
        }
    }
}

static void sc_tick_smart(struct CmpAiTacticalStatus *ts, double dt)
{
    soul_tick(&soul, ts, dt);
}

static void sc_tick(double dt)
{
    struct CmpAiTacticalStatus ts;

    sc_tick_camera(dt);
    sc_tick_dumb(dt);
    ts.hunter_pos = cmp_ori_get(hunter.ori);
    sc_tick_smart(&ts, dt);

    col_handle_all(&hunter, &lvl);
}

static void sc_draw_debug_graph(void)
{
    int i;
    char buf[10];

    for (i = 0; i < lgph.nodes_count; ++i) {

        struct Adj *adj;
        struct TilePos atp = lgph.nodes[i];
        struct WorldPos awp = pos_tile_to_world(atp);
        struct ScreenPos asp = pos_world_to_screen(awp);
        double x1 = asp.x + sc_tile_w / 2.0;
        double y1 = asp.y + sc_tile_w / 2.0;

        for (adj = lgph.adjacency[i]; adj->neighbor != -1; ++adj) {

            struct TilePos btp = lgph.nodes[adj->neighbor];
            struct WorldPos bwp = pos_tile_to_world(btp);
            struct ScreenPos bsp = pos_world_to_screen(bwp);
            double x2 = bsp.x + sc_tile_w / 2.0;
            double y2 = bsp.y + sc_tile_w / 2.0;

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

    cmp_drv_waypoint_points(soul->drv, &points, &points_count);

    for (i = 0; i < points_count; ++i) {

        struct WorldPos
            wp1 = { points[2 * i + 0], points[2 * i + 1] },
            wp2 = { points[2 * i + 2], points[2 * i + 3] };

        struct ScreenPos
            sp1 = pos_world_to_screen(wp1),
            sp2 = pos_world_to_screen(wp2);

        double x1 = sp1.x + sc_tile_w / 2.0;
        double y1 = sp1.y + sc_tile_w / 2.0;
        double x2 = sp2.x + sc_tile_w / 2.0;
        double y2 = sp2.y + sc_tile_w / 2.0;

        al_draw_line(x1, y1, x2, y2, color, 2.0);
    }
}

static void sc_draw(double weight)
{
    int i;
    struct WorldPos zero_wp = { 0.0, 0.0 };
    struct ScreenPos zero_sp = pos_world_to_screen(zero_wp);

    al_clear_to_color(al_map_rgb_f(0.0, 0.0, 0.0));
    lvl_draw(&lvl);
    hunter_draw(&hunter);
    soul_draw(&soul);

    for (i = 0; i < arrows.size; ++i) {
        cmp_draw(
            arrows.data[i].ori,
            arrows.data[i].appr,
            -zero_sp.x, -zero_sp.y);
    }

    if (sys_keys[ALLEGRO_KEY_F1]) {
        col_draw_last();
    }
    // if (sys_keys[ALLEGRO_KEY_F2]) {
        sc_draw_debug_graph();
        sc_draw_debug_soul(&soul);
    //}

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

