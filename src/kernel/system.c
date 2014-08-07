/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <stdio.h>

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_primitives.h>

#include "diagnostics.h"
#include "database.h"
#include "system.h"

bool sys_keys[ALLEGRO_KEY_MAX];

static ALLEGRO_DISPLAY *display;
static ALLEGRO_EVENT_QUEUE *ev_queue;

static int screen_w;
static int screen_h;
static double fps;
static double spf;
static double current_time;
static double accumulator;

static void sys_process_events(struct SysClient *client)
{
    ALLEGRO_EVENT event;

    while (!al_is_event_queue_empty(ev_queue)) {
        al_get_next_event(ev_queue, &event);
        switch (event.type) {
        case ALLEGRO_EVENT_DISPLAY_CLOSE:
            client->alive = false;
            return;

        case ALLEGRO_EVENT_KEY_DOWN:
            sys_keys[event.keyboard.keycode] = true;
            client->key(event.keyboard.keycode, true);
            break;

        case ALLEGRO_EVENT_KEY_UP:
            sys_keys[event.keyboard.keycode] = false;
            client->key(event.keyboard.keycode, false);
            break;

        default:
            break;
        }
    }
}

static void sys_realtime_loop_step(struct SysClient *client)
{
    static double max_frame_time = 0.25;

    double new_time = al_get_time();
    double frame_time = new_time - current_time;
    double frame_weight;

    if (frame_time > max_frame_time) {
        frame_time = max_frame_time;
    }

    current_time = new_time;
    accumulator += frame_time;

    while (accumulator >= spf) {
        client->tick(spf);
        if (!(*client->alive)) {
            return;
        }
        accumulator -= spf;
    }

    frame_weight = accumulator / spf;
    client->draw(frame_weight);
}

void sys_init(void)
{
    if (!al_init()) {
        DIAG_ERROR("Failed initializing platform wrapper.\n");
    }

    if (!al_init_image_addon()) {
        DIAG_ERROR("Failed initializing image addon.\n");
    }

    al_init_font_addon();
    if (!al_init_ttf_addon()) {
        DIAG_ERROR("Failed initializing TTF addon.\n");
    }

    if(!al_init_acodec_addon()) {
        DIAG_ERROR("Failed initializing acodec add-on.\n");
    }

    if(!al_init_primitives_addon()) {
        DIAG_ERROR("Failed initializing primitives add-on.\n");
    }

    screen_w = db_integer("screen_w");
    screen_h = db_integer("screen_h");
    fps = db_real("fps");
    spf = 1.0 / fps;

    display = al_create_display(screen_w, screen_h);
    if (!display) {
        DIAG_ERROR("Failed creating display.\n");
    }

    if (!al_install_keyboard()) {
        DIAG_ERROR("Failed installing keyboard.\n");
    }

    if(!al_install_audio()) {
        DIAG_ERROR("Failed initializing audio.\n");
    }

    ev_queue = al_create_event_queue();
    if (!ev_queue) {
        DIAG_ERROR("Failed creating event queue.\n");
    }

    al_register_event_source(ev_queue, al_get_display_event_source(display));
    al_register_event_source(ev_queue, al_get_keyboard_event_source());
}

void sys_realtime_loop(struct SysClient *client)
{
    client->init();

    current_time = al_get_time();
    accumulator = 0;

    while (true) {
        sys_process_events(client);
        if (!(*client->alive)) {
            return;
        }

        sys_realtime_loop_step(client);
        if (!(*client->alive)) {
            return;
        }

        al_rest(0.001);
    }
}

void sys_deinit(void)
{
    al_destroy_event_queue(ev_queue);
    al_destroy_display(display);
}

