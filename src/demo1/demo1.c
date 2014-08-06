/* Copyright (C) 2014 Krzysztof Stachowiak */

#include "resources.h"
#include "cmp_appr.h"
#include "cmp_ori.h"
#include "cmp_drv.h"
#include "cmp_operations.h"
#include "play.h"
#include "database.h"
#include "menu.h"
#include "demo1.h"

/* Local state. */
static bool demo1_alive;
static struct SysClient *demo1_next;

static struct CmpAppr *ball_appr;
static struct CmpOri *ball_ori;
static struct CmpDrv *ball_drv;
static double bounce_period;
static double bounce_accumulator;

static struct CmpAppr *tank_appr;
static struct CmpOri *tank_ori;
static struct CmpDrv *tank_drv;
static int x_drive, y_drive;

/* Resource handles. */
static void *tank_sprite;
static void *ball_sheet;
static void *bounce_sample;
static int screen_w;

static void demo1_init_ball(void)
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
        frames, frames_count,
        frame_indices, frame_times, frame_defs_count,
        frame_w, 3, -1);

    ball_ori = cmp_ori_create(100.0, 200.0, 0.0);
    ball_drv = cmp_drv_create_linear(false, 200.0, 0.0, 0.0);

    bounce_sample = res_load_sample("data/BounceYoFrankie.ogg");
    bounce_period = frame_count * frame_time;
    bounce_accumulator = 0.0;
}

static void demo1_init_tank(void)
{
    tank_sprite = res_load_bitmap("data/KV-2.png");

    tank_appr = cmp_appr_create_static_sprite(tank_sprite);
    tank_ori = cmp_ori_create(400.0, 400.0, 0);
    tank_drv = cmp_drv_create_input_8dir(true, 100, &x_drive, &y_drive);

    x_drive = 0;
    y_drive = 0;
}

static void demo1_tick_ball(double dt)
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

static void demo1_tick_tank(double dt)
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

/* Client API.
 * ===========
 */

static void demo1_init(void)
{
    screen_w = db_integer("screen_w");

    demo1_alive = true;
    demo1_next = NULL;

    demo1_init_ball();
    demo1_init_tank();
}

static void demo1_deinit(void)
{
    cmp_appr_free(ball_appr);
    cmp_ori_free(ball_ori);
    cmp_drv_free(ball_drv);

    cmp_appr_free(tank_appr);
    cmp_ori_free(tank_ori);
    cmp_drv_free(tank_drv);
}

static void demo1_tick(double dt)
{
    demo1_tick_ball(dt);
    demo1_tick_tank(dt);
}

static void demo1_draw(double weight)
{
    al_clear_to_color(al_map_rgb_f(0.25, 0.25, 0.25));
    cmp_draw(ball_ori, ball_appr);
    cmp_draw(tank_ori, tank_appr);
    al_flip_display();
}

static void demo1_key(int key, bool down)
{
    if (down && key == ALLEGRO_KEY_ESCAPE) {
        demo1_next = menu_get_client();
        demo1_alive = false;
    }
}

static struct SysClient demo1_client = {
    &demo1_alive,
    &demo1_next,
    &demo1_init,
    &demo1_deinit,
    &demo1_tick,
    &demo1_draw,
    &demo1_key
};

struct SysClient *demo1_get_client(void)
{
    return &demo1_client;
}

