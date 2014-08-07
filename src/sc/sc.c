/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <stdbool.h>

#include <allegro5/allegro_primitives.h>

#include "sc.h"
#include "sc_data.h"
#include "sc_hunter.h"
#include "sc_level.h"
#include "resources.h"
#include "database.h"
#include "menu.h"
#include "draw.h"
#include "cmp_ori.h"
#include "cmp_drv.h"
#include "cmp_operations.h"

/* Local state. */
static bool sc_alive;
static struct SysClient *sc_next;

struct Level lvl;
struct Hunter hunter;

int minx, miny, maxx, maxy;

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

static void sc_handle_collisions(void)
{
    int x, y;
    bool collided = false;
    bool floor_collided = false;
    struct PosRot hunter_pr = cmp_ori_get(hunter.ori);
    struct AABB hunter_aabb = {
        hunter_pr.x - hunter.box_w / 2.0,
        hunter_pr.y - hunter.box_h / 2.0,
        hunter_pr.x + hunter.box_w / 2.0,
        hunter_pr.y + hunter.box_h / 2.0
    };

    sc_last_hunter_aabb = hunter_aabb;

    minx = (hunter_pr.x / sc_tile_w) - 1;
    maxx = minx + 2;
    miny = (hunter_pr.y / sc_tile_w) - 1;
    maxy = miny + 2;


    for (x = minx; x <= maxx; ++x) {
        for (y = miny; y <= maxy; ++y) {

            struct TilePos tp;
            struct WorldPos wp;
            struct AABB tile_aabb;
            int c = lvl_get_tile(&lvl, x, y);

            if (c != '#' || (hunter.standing && y == maxy)) {
                continue;
            }

            tp.x = x;
            tp.y = y;
            wp = pos_tile_to_world(tp);
            tile_aabb.ax = wp.x;
            tile_aabb.ay = wp.y;
            tile_aabb.bx = wp.x + sc_tile_w;
            tile_aabb.by = wp.y + sc_tile_w;

            if (aabb_aabb(tile_aabb, hunter_aabb)) {
                collided = true;
                if (y == maxy) {
                    floor_collided = true;
                }
            }
        }
    }

    if (collided) {
        cmp_ori_cancel_x(hunter.ori);
        cmp_ori_cancel_y(hunter.ori);
        cmp_drv_stop(hunter.drv);
    }

    if (floor_collided) {
        hunter.standing = true;
    }
}

/* Client API.
 * ===========
 */

static void sc_init(void)
{
    sc_rock_tile = res_load_bitmap("data/rock_tile.png");
    sc_hunter_stand = res_load_bitmap("data/hunter_stand.png");

    sc_alive = true;
    sc_next = NULL;

    sc_screen_w = db_integer("screen_w");
    sc_screen_h = db_integer("screen_h");
    sc_tile_w = 64;

    lvl_load(&lvl, "data/map");
    hunter_init(&hunter, sc_hunter_stand);

    sc_cam_shift.x = 0.0;
    sc_cam_shift.y = 0.0;
    sc_update_screen_aabb();
}

static void sc_deinit(void)
{
    lvl_unload(&lvl);
}

static void sc_tick(double dt)
{
    sc_tick_camera(dt);
    hunter_tick(&hunter, dt);
    sc_handle_collisions();

    hunter.inx = sys_keys[ALLEGRO_KEY_RIGHT] - sys_keys[ALLEGRO_KEY_LEFT];
    hunter.jump_req = sys_keys[ALLEGRO_KEY_UP];
}

static void sc_draw(double weight)
{
    al_clear_to_color(al_map_rgb_f(0.0, 0.0, 0.0));
    lvl_draw(&lvl);
    hunter_draw(&hunter);

    /* begin debug */
    double x1, y1, x2, y2;
    struct AABB test_aabb = {
        minx * sc_tile_w,
        miny * sc_tile_w,
        maxx * sc_tile_w,
        maxy * sc_tile_w
    };
    aabb_to_screen(test_aabb, &x1, &y1, &x2, &y2);
    al_draw_rectangle(x1, y1, x2, y2, al_map_rgb_f(1, 0, 0), 1.0);
    /* end debug */

    al_flip_display();
}

static void sc_key(int key, bool down)
{
    if (down && key == ALLEGRO_KEY_ESCAPE) {
        sc_next = menu_get_client();
        sc_alive = false;
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

