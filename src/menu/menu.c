#include <allegro5/allegro.h>

#include "menu.h"

static bool menu_alive;
static bool menu_redraw;
static bool dummy_flag;

static void menu_init(void)
{
    menu_alive = true;
    menu_redraw = true;
    dummy_flag = false;
}

static void menu_deinit(void)
{
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
    al_flip_display();
}

static void menu_key(int key, bool down)
{
    if (down && key == ALLEGRO_KEY_DOWN) {
        dummy_flag = !dummy_flag;
        menu_redraw = true;
    } else if (down && key == ALLEGRO_KEY_ESCAPE) {
        menu_alive = false;
    }
}

static struct SysClient menu_client = {
    &menu_alive,
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

