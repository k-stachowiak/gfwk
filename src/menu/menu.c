/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <stdio.h>

#include <allegro5/allegro.h>

#include "diagnostics.h"
#include "string_util.h"
#include "menu.h"
#include "menu_struct.h"
#include "resources.h"
#include "database.h"
#include "demo.h"
#include "draw.h"
#include "play.h"
#include "sc.h"

/* Configuration cache. */
static double menu_text_caption_r;
static double menu_text_caption_g;
static double menu_text_caption_b;
static double menu_text_on_r;
static double menu_text_on_g;
static double menu_text_on_b;
static double menu_text_off_r;
static double menu_text_off_g;
static double menu_text_off_b;
static long menu_text_size;
static double menu_text_y_offset;

/* Local state. */
static bool menu_alive;
static struct SysClient *menu_next;
static bool menu_redraw;
static long dummy_flag;
static struct Menu *main_menu;

/* Resource handles. */
static void *menu_font;
static void *nav_move_sample;
static void *nav_enter_sample;
static int screen_w;

static void menu_demo_callback(void)
{
    menu_next = demo_get_client();
    menu_alive = false;
}

static void menu_game_callback(void)
{
    menu_next = sc_get_client();
    menu_alive = false;
}

static void menu_quit_callback(void)
{
    menu_next = NULL;
    menu_alive = false;
}

static struct MenuPage *menu_create_options_page(void)
{
    struct MenuItem *mi_dummy_flag = menu_item_create_value(
            "Dummy flag",
            &dummy_flag,
            NULL);

    return menu_page_create("+ Options menu", mi_dummy_flag);
}

static struct MenuPage *menu_create_main_page(struct MenuPage *options_page)
{
    struct MenuItem *mi_demo = menu_item_create_action(
            "Demo",
            menu_demo_callback);

    struct MenuItem *mi_game = menu_item_create_action(
            "Game",
            menu_game_callback);

    struct MenuItem *mi_options = menu_item_create_ref(
            "Options",
            options_page);

    struct MenuItem *mi_quit = menu_item_create_action(
            "Quit",
            menu_quit_callback);

    mi_demo->next = mi_game;
    mi_game->next = mi_options;
    mi_options->next = mi_quit;

    return menu_page_create("+ Main menu", mi_demo);
}

void menu_draw_page(struct MenuPage *page)
{
    double x = (double)screen_w / 2.0;
    double y = 170.0;
    double r, g, b;
    int index = 0;
    int cap_len;
    char text[4096] = { 0 }; /* TODO: Fix this. */
    struct MenuItem *item = page->items;

    draw_text(
        menu_font, x, y,
        menu_text_caption_r,
        menu_text_caption_g,
        menu_text_caption_b,
        1,
        false,
        page->caption);

    while (item) {

        if (index++ == page->current_item) {
            r = menu_text_on_r;
            g = menu_text_on_g;
            b = menu_text_on_b;
        } else {
            r = menu_text_off_r;
            g = menu_text_off_g;
            b = menu_text_off_b;
        }

        cap_len = strlen(item->caption);
        if (item->type == MENU_ITEM_VALUE) {
            if (item->body.ptr.integer) {
                long val = *item->body.ptr.integer;
                sprintf(text, "%s (%ld)", item->caption, val);

            } else if (item->body.ptr.real) {
                double val = *item->body.ptr.real;
                sprintf(text, "%s (%f)", item->caption, val);

            } else {
                DIAG_ERROR("Unhandled value menu item type.");
                exit(1);
            }
        } else {
            memcpy(text, item->caption, cap_len + 1);
        }

        y += menu_text_y_offset;
        draw_text(menu_font, x, y, r, g, b, 1, true, text);
        item = item->next;
    }
}

/* Client API.
 * ===========
 */

static void menu_init(void)
{
    struct MenuPage *mp_opts;
    struct MenuPage *mp_main;

    menu_text_caption_r = db_real("menu_text_caption_r");
    menu_text_caption_g = db_real("menu_text_caption_g");
    menu_text_caption_b = db_real("menu_text_caption_b");
    menu_text_on_r = db_real("menu_text_on_r");
    menu_text_on_g = db_real("menu_text_on_g");
    menu_text_on_b = db_real("menu_text_on_b");
    menu_text_off_r = db_real("menu_text_off_r");
    menu_text_off_g = db_real("menu_text_off_g");
    menu_text_off_b = db_real("menu_text_off_b");
    menu_text_size = db_integer("menu_text_size");
    menu_text_y_offset = db_real("menu_text_y_offset");

    menu_alive = true;
    menu_next = NULL;
    menu_redraw = true;
    dummy_flag = 0;

    screen_w = db_integer("screen_w");

    mp_opts = menu_create_options_page();
    mp_main = menu_create_main_page(mp_opts);

    main_menu = menu_create(mp_main);

    menu_font = res_load_font("data/prstartk.ttf", menu_text_size);
    nav_enter_sample = res_load_sample("data/beep.ogg");
    nav_move_sample = res_load_sample("data/MenuSelectionClick.ogg");

}

static void menu_deinit(void)
{
    menu_free(main_menu);
}

static void menu_tick(double dt)
{
    (void)dt;
    return;
}

static void menu_draw(double weight)
{
    ALLEGRO_COLOR color;

    (void)weight;

    if (!menu_redraw) {
        return;
    } else {
        menu_redraw = false;
    }

    if (dummy_flag) {
        color = al_map_rgb_f(0.0, 0.0, 0.0);
    } else {
        color = al_map_rgb_f(0.25, 0.25, 0.25);
    }

    al_clear_to_color(color);
    menu_draw_page(main_menu->current_page);
    al_flip_display();
}

static void menu_key(int key, bool down)
{
    if (down && key == ALLEGRO_KEY_UP) {
        menu_nav_up(main_menu);
        play_sample(nav_move_sample);
    } else if (down && key == ALLEGRO_KEY_DOWN) {
        menu_nav_down(main_menu);
        play_sample(nav_move_sample);
    } else if (down && key == ALLEGRO_KEY_ENTER) {
        menu_nav_enter(main_menu);
        play_sample(nav_enter_sample);
    } else if (down && key == ALLEGRO_KEY_ESCAPE) {
        menu_nav_back(main_menu);
        play_sample(nav_enter_sample);
    } else if (down && (
            key == ALLEGRO_KEY_MINUS ||
            key == ALLEGRO_KEY_PAD_MINUS)) {
        menu_input_decr(main_menu);
        play_sample(nav_move_sample);
    } else if (down && (
            key == ALLEGRO_KEY_EQUALS ||
            key == ALLEGRO_KEY_PAD_PLUS)) {
        menu_input_incr(main_menu);
        play_sample(nav_move_sample);
    }
    menu_redraw = true;
}

static struct SysClient menu_client = {
    &menu_alive,
    &menu_next,
    &menu_init,
    &menu_deinit,
    &menu_tick,
    &menu_draw,
    &menu_key
};

struct SysClient *menu_get_client(void)
{
    return &menu_client;
}

