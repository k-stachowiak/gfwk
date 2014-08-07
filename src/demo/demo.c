/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <math.h>

#include "array.h"
#include "resources.h"
#include "cmp_appr.h"
#include "cmp_ori.h"
#include "cmp_drv.h"
#include "cmp_operations.h"
#include "play.h"
#include "database.h"
#include "menu.h"
#include "demo.h"

/* Local state. */
static bool demo_alive;
static struct SysClient *demo_next;

static struct CmpAppr *ball_appr;
static struct CmpOri *ball_ori;
static struct CmpDrv *ball_drv;
static double bounce_period;
static double bounce_accumulator;

static struct CmpAppr *tank_appr;
static struct CmpOri *tank_ori;
static struct CmpDrv *tank_drv;
static int x_drive, y_drive;

struct Bullet {
    struct CmpAppr *appr;
    struct CmpOri *ori;
    struct CmpDrv *drv;
};

struct {
    struct Bullet *data;
    int size, cap;
} bullets;

/* Resource handles. */
static void *tank_sprite;
static void *ball_sheet;
static void *bullet_sprite;
static void *bounce_sample;
static void *shot_sample;
static int screen_w;
static int screen_h;

static void bullet_fire(void)
{
    struct PosRot tank_pr;
    struct Bullet bullet;
    double bullet_vel = 400.0;
    double dir_x, dir_y;

    tank_pr = cmp_ori_get(tank_ori);
    dir_x = cos(tank_pr.theta);
    dir_y = sin(tank_pr.theta);

    bullet.appr = cmp_appr_create_static_sprite(0.0, 0.0, bullet_sprite);
    bullet.ori = cmp_ori_create(tank_pr.x, tank_pr.y, tank_pr.theta);
    bullet.drv = cmp_drv_create_linear(
            true, dir_x * bullet_vel, dir_y * bullet_vel, 0.0);

    ARRAY_APPEND(bullets, bullet);

    play_sample(shot_sample);
}

static void bullet_free(int i)
{
    cmp_appr_free(bullets.data[i].appr);
    cmp_ori_free(bullets.data[i].ori);
    cmp_drv_free(bullets.data[i].drv);
}

static void demo_init_ball(void)
{
    int i;

    void **frames;
    int frames_count;

    int *frame_indices;
    double *frame_times;
    int frame_defs_count;

    int frame_w = 50;
    int frame_count = 6;
    double frame_time = 0.1;

    ball_sheet = res_load_bitmap("data/ball.png");
    res_cut_frame_sheet(ball_sheet, frame_w, &frames, &frames_count);

    frame_defs_count = frame_count;
    frame_indices = malloc(frame_defs_count * sizeof(*frame_indices));
    frame_times = malloc(frame_defs_count * sizeof(*frame_times));

    for (i = 0; i < frame_defs_count; ++i) {
        frame_indices[i] = i;
        frame_times[i] = frame_time;
    }

    ball_appr = cmp_appr_create_anim_sprite(
        0.0, 0.0,
        frames, frames_count,
        frame_indices, frame_times, frame_defs_count,
        frame_w, 3, -1);

    ball_ori = cmp_ori_create(100.0, 200.0, 0.0);
    ball_drv = cmp_drv_create_linear(false, 200.0, 0.0, 0.0);

    bounce_sample = res_load_sample("data/BounceYoFrankie.ogg");
    bounce_period = frame_count * frame_time;
    bounce_accumulator = 0.0;
}

static void demo_init_tank(void)
{
    tank_sprite = res_load_bitmap("data/KV-2.png");

    tank_appr = cmp_appr_create_static_sprite(0.0, 0.0, tank_sprite);
    tank_ori = cmp_ori_create(400.0, 400.0, 0);
    tank_drv = cmp_drv_create_input_8dir(true, 100, &x_drive, &y_drive);

    x_drive = 0;
    y_drive = 0;

    shot_sample = res_load_sample("data/cg1.ogg");
}

static void demo_init_bullets(void)
{
    bullets.data = NULL;
    bullets.size = 0;
    bullets.cap = 0;

    bullet_sprite = res_load_bitmap("data/bullet.png");
}

static void demo_tick_ball(double dt)
{
    struct PosRot ball_pr;

    cmp_appr_update(ball_appr, dt);
    cmp_drive(ball_ori, ball_drv, dt);

    ball_pr = cmp_ori_get(ball_ori);
    if (ball_pr.x < 0.0 || ball_pr.x > screen_w) {
        ball_drv->body.lin.vx *= -1.0;
        play_sample(bounce_sample);
    }

    bounce_accumulator -= dt;
    if (bounce_accumulator <= 0.0) {
        bounce_accumulator += bounce_period;
        play_sample(bounce_sample);
    }
}

static void demo_tick_tank(double dt)
{
    x_drive = 0;
    if (sys_keys[ALLEGRO_KEY_LEFT]) {
        x_drive -= 1;
    }
    if (sys_keys[ALLEGRO_KEY_RIGHT]) {
        x_drive += 1;
    }

    y_drive = 0;
    if (sys_keys[ALLEGRO_KEY_UP]) {
        y_drive -= 1;
    }
    if (sys_keys[ALLEGRO_KEY_DOWN]) {
        y_drive += 1;
    }

    cmp_drive(tank_ori, tank_drv, dt);
}

static void demo_tick_bullets(double dt)
{
    int i;
    int margin = 20;
    for (i = 0; i < bullets.size; ++i) {
        struct PosRot pr;
        cmp_drive(bullets.data[i].ori, bullets.data[i].drv, dt);
        pr = cmp_ori_get(bullets.data[i].ori);
        if (pr.x < margin || pr.x > (screen_w - margin) ||
            pr.y < margin || pr.y > (screen_h - margin)) {
                ARRAY_REMOVE(bullets, i);
                --i;
        }
    }
}

/* Client API.
 * ===========
 */

static void demo_init(void)
{
    screen_w = db_integer("screen_w");
    screen_h = db_integer("screen_h");

    demo_alive = true;
    demo_next = NULL;

    demo_init_ball();
    demo_init_tank();
    demo_init_bullets();
}

static void demo_deinit(void)
{
    int i;

    cmp_appr_free(ball_appr);
    cmp_ori_free(ball_ori);
    cmp_drv_free(ball_drv);

    cmp_appr_free(tank_appr);
    cmp_ori_free(tank_ori);
    cmp_drv_free(tank_drv);

    for (i = 0; i < bullets.size; ++i) {
        bullet_free(i);
    }
    ARRAY_FREE(bullets);
}

static void demo_tick(double dt)
{
    int i;

    demo_tick_ball(dt);
    demo_tick_tank(dt);
    demo_tick_bullets(dt);

    if (cmp_ori_distance(ball_ori, tank_ori) < 100.0) {
        goto termination;
    }

    for (i = 0; i < bullets.size; ++i) {
        if (cmp_ori_distance(ball_ori, bullets.data[i].ori) < 100.0) {
            goto termination;
        }
    }

    return;

termination:
    demo_next = menu_get_client();
    demo_alive = false;
}

static void demo_draw(double weight)
{
    int i;

    al_clear_to_color(al_map_rgb_f(0.25, 0.25, 0.25));
    cmp_draw(ball_ori, ball_appr, 0.0, 0.0);
    cmp_draw(tank_ori, tank_appr, 0.0, 0.0);
    for (i = 0; i < bullets.size; ++i) {
        cmp_draw(bullets.data[i].ori, bullets.data[i].appr, 0.0, 0.0);
    }
    al_flip_display();
}

static void demo_key(int key, bool down)
{
    if (down && key == ALLEGRO_KEY_SPACE) {
        bullet_fire();
    } else if (down && key == ALLEGRO_KEY_ESCAPE) {
        demo_next = menu_get_client();
        demo_alive = false;
    }
}

static struct SysClient demo_client = {
    &demo_alive,
    &demo_next,
    &demo_init,
    &demo_deinit,
    &demo_tick,
    &demo_draw,
    &demo_key
};

struct SysClient *demo_get_client(void)
{
    return &demo_client;
}

