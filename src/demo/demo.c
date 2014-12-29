/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <math.h>

#include "array.h"
#include "memory.h"
#include "resources.h"
#include "cmp_appr.h"
#include "cmp_ori.h"
#include "cmp_drv.h"
#include "cmp_operations.h"
#include "play.h"
#include "database.h"
#include "menu.h"
#include "demo.h"

/* Configuration cache. */
static int screen_w;
static int screen_h;

/* Local state. */
static bool demo_alive;
static struct SysClient *demo_next;

/* Basic resources. */
static void *ball_sheet;
static void *tank_sprite;
static void *bullet_sprite;
static void *bounce_sample;
static void *shot_sample;

/* Complex resources. */
struct CmpApprAnimSpriteCommon *ball_anim_common;

/* Entities. */
struct Ball {
    struct CmpAppr appr;
    struct CmpOri ori;
    struct CmpDrv drv;
    double bounce_period;
    double bounce_accumulator;
};
static struct Ball ball;

struct Tank {
    struct CmpAppr appr;
    struct CmpOri ori;
    struct CmpDrv drv;
    int x_drive;
    int y_drive;
};
static struct Tank tank;

struct Bullet {
    struct CmpAppr appr;
    struct CmpOri ori;
    struct CmpDrv drv;
};
struct { struct Bullet *data; int size, cap; } bullets;

/* Common logic. */
static void demo_init_resources_basic(void)
{
    ball_sheet = res_load_bitmap("data/ball.png");
    bounce_sample = res_load_sample("data/BounceYoFrankie.ogg");
    tank_sprite = res_load_bitmap("data/KV-2.png");
    shot_sample = res_load_sample("data/cg1.ogg");
    bullet_sprite = res_load_bitmap("data/bullet.png");
}

static void demo_deinit_resources_basic(void)
{
    res_dispose_bitmap(ball_sheet);
    res_dispose_sample(bounce_sample);
    res_dispose_bitmap(tank_sprite);
    res_dispose_sample(shot_sample);
    res_dispose_bitmap(bullet_sprite);
}

static void demo_init_resources_complex(void)
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

    res_cut_frame_sheet(ball_sheet, frame_w, &frames, &frames_count);
    frame_defs_count = frame_count;
    frame_indices = malloc_or_die(frame_defs_count * sizeof(*frame_indices));
    frame_times = malloc_or_die(frame_defs_count * sizeof(*frame_times));
    for (i = 0; i < frame_defs_count; ++i) {
        frame_indices[i] = i;
        frame_times[i] = frame_time;
    }

    ball_anim_common = cmp_appr_anim_sprite_common_create(
            frames, frames_count,
            frame_indices, frame_times, frame_defs_count,
            frame_w);
}

static void demo_deinit_resources_complex(void)
{
    cmp_appr_anim_sprite_common_free(ball_anim_common);
}

/* Ball logic. */
static void demo_init_ball(struct Ball *b)
{
    cmp_appr_anim_sprite_init(&b->appr, ball_anim_common, 3, -1);
	cmp_ori_init(&b->ori, 100.0, 200.0, 0.0);
	cmp_drv_linear_init(&b->drv, false, 200.0, 0.0, 0.0);

    b->bounce_period =
        ball_anim_common->frames_count *
        ball_anim_common->frame_times[0];

    b->bounce_accumulator = 0.0;
}

static void demo_deinit_ball(struct Ball *b)
{
	cmp_drv_deinit(&b->drv);
	cmp_appr_deinit(&b->appr);
    cmp_ori_deinit(&b->ori);
}

static void demo_tick_ball(double dt)
{
    struct PosRot ball_pr;
    struct Vel ball_v;

	cmp_appr_update(&ball.appr, dt);
    cmp_drive(&ball.ori, &ball.drv, dt);

    ball_pr = cmp_ori_get(&ball.ori);
    ball_v = cmp_drv_vel(&ball.drv);
    if (ball_pr.x < 0.0 || ball_pr.x > screen_w) {
        ball_v.vx *= -1.0;
        play_sample(bounce_sample);
    }

    ball.bounce_accumulator -= dt;
    if (ball.bounce_accumulator <= 0.0) {
        ball.bounce_accumulator += ball.bounce_period;
        play_sample(bounce_sample);
    }
}

/* Tank logic. */
static void demo_init_tank(struct Tank *t)
{
	cmp_appr_static_sprite_init(&t->appr, tank_sprite);
	cmp_ori_init(&t->ori, 400.0, 400.0, 0);
	cmp_drv_i8d_init(&t->drv, true, 100, &t->x_drive, &t->y_drive);

    t->x_drive = 0;
    t->y_drive = 0;
}

static void demo_deinit_tank(struct Tank *t)
{
	cmp_drv_deinit(&t->drv);
	cmp_appr_deinit(&t->appr);
    cmp_ori_deinit(&t->ori);
}

static void demo_tick_tank(double dt)
{
    tank.x_drive = 0;
    if (sys_keys[ALLEGRO_KEY_LEFT]) {
        tank.x_drive -= 1;
    }
    if (sys_keys[ALLEGRO_KEY_RIGHT]) {
        tank.x_drive += 1;
    }

    tank.y_drive = 0;
    if (sys_keys[ALLEGRO_KEY_UP]) {
        tank.y_drive -= 1;
    }
    if (sys_keys[ALLEGRO_KEY_DOWN]) {
        tank.y_drive += 1;
    }

    cmp_drive(&tank.ori, &tank.drv, dt);
}

/* Bullet logic. */
static void demo_init_bullets(void)
{
    bullets.data = NULL;
    bullets.size = 0;
    bullets.cap = 0;
}

static void demo_deinit_bullet(int i)
{
	cmp_drv_deinit(&bullets.data[i].drv);
	cmp_appr_deinit(&bullets.data[i].appr);
    cmp_ori_deinit(&bullets.data[i].ori);
}

static void demo_deinit_bullets(void)
{
    int i;
    for (i = 0; i < bullets.size; ++i) {
        demo_deinit_bullet(i);
    }
    ARRAY_FREE(bullets);
}

static void demo_tick_bullets(double dt)
{
    int i;
    int margin = 20;
    for (i = 0; i < bullets.size; ++i) {
        struct PosRot pr;
        cmp_drive(&bullets.data[i].ori, &bullets.data[i].drv, dt);
        pr = cmp_ori_get(&bullets.data[i].ori);
        if (pr.x < margin || pr.x > (screen_w - margin) ||
            pr.y < margin || pr.y > (screen_h - margin)) {
                demo_deinit_bullet(i);
                ARRAY_REMOVE(bullets, i);
                --i;
        }
    }
}

static void bullet_fire(void)
{
    struct PosRot tank_pr;
    struct Bullet bullet;
    double bullet_vel = 400.0;
    double dir_x, dir_y;

    tank_pr = cmp_ori_get(&tank.ori);
    dir_x = cos(tank_pr.theta);
    dir_y = sin(tank_pr.theta);

	cmp_appr_static_sprite_init(&bullet.appr, bullet_sprite);
	cmp_ori_init(&bullet.ori, tank_pr.x, tank_pr.y, tank_pr.theta);
	cmp_drv_linear_init(
		&bullet.drv,
        true,
		dir_x * bullet_vel,
		dir_y * bullet_vel,
		0.0);

    ARRAY_APPEND(bullets, bullet);

    play_sample(shot_sample);
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

    demo_init_resources_basic();
    demo_init_resources_complex();

    demo_init_ball(&ball);
    demo_init_tank(&tank);
    demo_init_bullets();
}

static void demo_deinit(void)
{
    demo_deinit_bullets();
    demo_deinit_tank(&tank);
    demo_deinit_ball(&ball);

    demo_deinit_resources_complex();
    demo_deinit_resources_basic();
}

static void demo_tick(double dt)
{
    int i;

    demo_tick_ball(dt);
    demo_tick_tank(dt);
    demo_tick_bullets(dt);

    if (cmp_ori_distance(&ball.ori, &tank.ori) < 100.0) {
        goto termination;
    }

    for (i = 0; i < bullets.size; ++i) {
        if (cmp_ori_distance(&ball.ori, &bullets.data[i].ori) < 100.0) {
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
    cmp_draw(&ball.ori, &ball.appr, 0.0, 0.0);
    cmp_draw(&tank.ori, &tank.appr, 0.0, 0.0);
    for (i = 0; i < bullets.size; ++i) {
        cmp_draw(&bullets.data[i].ori, &bullets.data[i].appr, 0.0, 0.0);
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

