/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <allegro5/allegro_primitives.h>

#include "cmp_ori.h"
#include "cmp_drv.h"
#include "sc_collision.h"
#include "sc_data.h"

struct CollisionContext {
    struct AABB bbox;
    struct VLine lsline, rsline;
    int utiles[3];
    int btiles[3];
    int ltiles[3];
    int rtiles[3];
    struct AABB utile_aabbs[3];
    struct AABB btile_aabbs[3];
    struct AABB ltile_aabbs[3];
    struct AABB rtile_aabbs[3];
} cc_last;

static struct AABB col_tile_aabb(int x, int y)
{
    struct TilePos tp = { x, y };
    struct WorldPos wp = pos_tile_to_world(tp);
    struct AABB aabb = { wp.x, wp.y, wp.x + sc_tile_w, wp.y + sc_tile_w };
    return aabb;
}

static struct CollisionContext col_analyze(
        struct Hunter *hunter,
        struct Level *lvl)
{
    struct PosRot pr = cmp_ori_get(hunter->ori);
    double w = hunter->box_w;
    double h = hunter->box_h;
    struct TilePos tp = { pr.x / sc_tile_w, pr.y / sc_tile_w };
    struct CollisionContext result;

    /* Bottom collision box. */
    result.bbox.ax = pr.x - w / 2.0;
    result.bbox.ay = pr.y + h / 2.0 + 1.0;
    result.bbox.bx = pr.x + w / 2.0;
    result.bbox.by = pr.y + h / 2.0 + 3.0;

    /* Left and right scan lines. */
    result.lsline.x = pr.x - w / 2.0;
    result.lsline.y1 = pr.y - h / 2.0;
    result.lsline.y2 = pr.y + h / 2.0;
    result.rsline.x = pr.x + w / 2.0;
    result.rsline.y1 = pr.y - h / 2.0;
    result.rsline.y2 = pr.y + h / 2.0;

    /* Surrounding tile field values. */
    result.utiles[0] = lvl_get_tile(lvl, tp.x, tp.y - 1);
    result.utiles[1] = lvl_get_tile(lvl, tp.x - 1, tp.y - 1);
    result.utiles[2] = lvl_get_tile(lvl, tp.x + 1, tp.y - 1);
    result.btiles[0] = lvl_get_tile(lvl, tp.x, tp.y + 1);
    result.btiles[1] = lvl_get_tile(lvl, tp.x - 1, tp.y + 1);
    result.btiles[2] = lvl_get_tile(lvl, tp.x + 1, tp.y + 1);
    result.ltiles[0] = lvl_get_tile(lvl, tp.x - 1, tp.y);
    result.ltiles[1] = lvl_get_tile(lvl, tp.x - 1, tp.y - 1);
    result.ltiles[2] = lvl_get_tile(lvl, tp.x - 1, tp.y + 1);
    result.rtiles[0] = lvl_get_tile(lvl, tp.x + 1, tp.y);
    result.rtiles[1] = lvl_get_tile(lvl, tp.x + 1, tp.y - 1);
    result.rtiles[2] = lvl_get_tile(lvl, tp.x + 1, tp.y + 1);

    /* Surrounding tile bounding boxes. */
    result.utile_aabbs[0] = col_tile_aabb(tp.x, tp.y - 1);
    result.utile_aabbs[1] = col_tile_aabb(tp.x - 1, tp.y - 1);
    result.utile_aabbs[2] = col_tile_aabb(tp.x + 1, tp.y - 1);
    result.btile_aabbs[0] = col_tile_aabb(tp.x, tp.y + 1);
    result.btile_aabbs[1] = col_tile_aabb(tp.x - 1, tp.y + 1);
    result.btile_aabbs[2] = col_tile_aabb(tp.x + 1, tp.y + 1);
    result.ltile_aabbs[0] = col_tile_aabb(tp.x - 1, tp.y);
    result.ltile_aabbs[1] = col_tile_aabb(tp.x - 1, tp.y - 1);
    result.ltile_aabbs[2] = col_tile_aabb(tp.x - 1, tp.y + 1);
    result.rtile_aabbs[0] = col_tile_aabb(tp.x + 1, tp.y);
    result.rtile_aabbs[1] = col_tile_aabb(tp.x + 1, tp.y - 1);
    result.rtile_aabbs[2] = col_tile_aabb(tp.x + 1, tp.y + 1);

    return result;
}

static void col_handle_vertical(struct Hunter *hunter, struct CollisionContext *cc)
{
    if ((cc->ltiles[0] == '#' && aabb_vline(cc->ltile_aabbs[0], cc->lsline)) ||
        (cc->ltiles[1] == '#' && aabb_vline(cc->ltile_aabbs[1], cc->lsline)) ||
        (cc->ltiles[2] == '#' && aabb_vline(cc->ltile_aabbs[2], cc->lsline))) {
            cmp_drv_stop_x(hunter->drv);
            hunter->ori->current.x =
                cc->ltile_aabbs[0].bx +
                hunter->box_w / 2.0 +
                1.0;
    }
    if ((cc->rtiles[0] == '#' && aabb_vline(cc->ltile_aabbs[0], cc->rsline)) ||
        (cc->rtiles[1] == '#' && aabb_vline(cc->ltile_aabbs[1], cc->rsline)) ||
        (cc->rtiles[2] == '#' && aabb_vline(cc->ltile_aabbs[2], cc->rsline))) {
            cmp_drv_stop_x(hunter->drv);
            hunter->ori->current.x =
                cc->rtile_aabbs[0].ax -
                hunter->box_w / 2.0 -
                1.0;
    }
}

static void col_handle_standing(struct Hunter *hunter, struct CollisionContext *cc)
{
    if ((cc->btiles[0] != '#' || !aabb_aabb(cc->bbox, cc->btile_aabbs[0])) &&
        (cc->btiles[1] != '#' || !aabb_aabb(cc->bbox, cc->btile_aabbs[1])) &&
        (cc->btiles[2] != '#' || !aabb_aabb(cc->bbox, cc->btile_aabbs[2]))) {
            hunter->standing = false;
    }
}

static void col_handle_midair(struct Hunter *hunter, struct CollisionContext *cc)
{
    if ((cc->utiles[0] == '#' && (aabb_vline(cc->utile_aabbs[0], cc->lsline) ||
                                  aabb_vline(cc->utile_aabbs[0], cc->rsline))) ||
        (cc->utiles[1] == '#' && (aabb_vline(cc->utile_aabbs[1], cc->lsline) ||
                                  aabb_vline(cc->utile_aabbs[1], cc->rsline))) ||
        (cc->utiles[2] == '#' && (aabb_vline(cc->utile_aabbs[2], cc->lsline) ||
                                  aabb_vline(cc->utile_aabbs[2], cc->rsline)))) {
        cmp_drv_stop_y(hunter->drv);
        hunter->ori->current.y =
            cc->utile_aabbs[0].by +
            hunter->box_h / 2.0 +
            1.0;
    }

    if ((cc->btiles[0] == '#' && (aabb_vline(cc->btile_aabbs[0], cc->lsline) ||
                                  aabb_vline(cc->btile_aabbs[0], cc->rsline))) ||
        (cc->btiles[1] == '#' && (aabb_vline(cc->btile_aabbs[1], cc->lsline) ||
                                  aabb_vline(cc->btile_aabbs[1], cc->rsline))) ||
        (cc->btiles[2] == '#' && (aabb_vline(cc->btile_aabbs[2], cc->lsline) ||
                                  aabb_vline(cc->btile_aabbs[2], cc->rsline)))) {
            /* TODO: consider getting rid of the move cancelling operation. */
            hunter->standing = true;
            cmp_drv_stop_y(hunter->drv);
            hunter->ori->current.y =
                cc->btile_aabbs[0].ay -
                hunter->box_h / 2.0 -
                1.0;
    }
}

static void col_draw_aabb(struct AABB aabb, bool fill, double r, double g, double b)
{
    double x1, y1, x2, y2;
    aabb_to_screen(aabb, &x1, &y1, &x2, &y2);
    if (fill) {
        al_draw_filled_rectangle(x1, y1, x2, y2, al_map_rgb_f(r, g, b));
    } else {
        al_draw_rectangle(x1, y1, x2, y2, al_map_rgb_f(r, g, b), 1.0);
    }
}

static void col_draw_vline(struct VLine vline, double r, double g, double b)
{
    double x, y1, y2;
    vline_to_screen(vline, &x, &y1, &y2);
    al_draw_line(x, y1, x, y2, al_map_rgb_f(r, g, b), 1.0);
}

void col_handle_all(struct Hunter *hunter, struct Level *lvl)
{
    cc_last = col_analyze(hunter, lvl);

    col_handle_vertical(hunter, &cc_last);
    if (hunter->standing) {
        col_handle_standing(hunter, &cc_last);
    } else {
        col_handle_midair(hunter, &cc_last);
    }
}

void col_draw_last(void)
{
    col_draw_aabb(cc_last.bbox, true, 1, 1, 0);
    col_draw_vline(cc_last.lsline, 1, 1, 0);
    col_draw_vline(cc_last.rsline, 1, 1, 0);
    col_draw_aabb(cc_last.utile_aabbs[0], cc_last.utiles[0] == '#', 0, 1, 1);
    col_draw_aabb(cc_last.utile_aabbs[1], cc_last.utiles[1] == '#', 0, 1, 1);
    col_draw_aabb(cc_last.utile_aabbs[2], cc_last.utiles[2] == '#', 0, 1, 1);
    col_draw_aabb(cc_last.btile_aabbs[0], cc_last.btiles[0] == '#', 0, 1, 1);
    col_draw_aabb(cc_last.btile_aabbs[1], cc_last.btiles[1] == '#', 0, 1, 1);
    col_draw_aabb(cc_last.btile_aabbs[2], cc_last.btiles[2] == '#', 0, 1, 1);
    col_draw_aabb(cc_last.ltile_aabbs[0], cc_last.ltiles[0] == '#', 1, 0, 1);
    col_draw_aabb(cc_last.rtile_aabbs[0], cc_last.rtiles[0] == '#', 1, 0, 1);
}

