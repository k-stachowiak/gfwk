/* Copyright (C) 2014 Krzysztof Stachowiak */

#include "resources.h"
#include "cmp_appr.h"
#include "draw.h"
#include "play.h"
#include "menu.h"
#include "demo1.h"

/* Local state. */
static bool demo1_alive;
static struct SysClient *demo1_next;

void *ball_sheet;
static struct CmpAppr *ball_appr;

void *bounce_sample;
double bounce_period;
double bounce_accumulator;

static void ball_init(void)
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

    bounce_sample = res_load_sample("data/BounceYoFrankie.ogg");
    bounce_period = frame_count * frame_time;
    bounce_accumulator = 0.0;
}

/* Client API.
 * ===========
 */

static void demo1_init(void)
{
    demo1_alive = true;
    demo1_next = NULL;

    ball_init();
}

static void demo1_deinit(void)
{
    cmp_appr_free(ball_appr);
}

static void demo1_tick(double dt)
{
    cmp_appr_update(ball_appr, dt);

    bounce_accumulator -= dt;
    if (bounce_accumulator <= 0.0) {
        bounce_accumulator += bounce_period;
        play_sample(bounce_sample);
    }
}

static void demo1_draw(double weight)
{
    al_clear_to_color(al_map_rgb_f(0.25, 0.25, 0.25));
    draw_bitmap(cmp_appr_bitmap(ball_appr), 100, 100, 0);
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

