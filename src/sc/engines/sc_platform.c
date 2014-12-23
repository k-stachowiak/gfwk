/* Copyright (C) 2014 Krzysztof Stachowiak */

#include "sc_data.h"

#include "sc_collision.h"

#include "sc_hunter.h"
#include "sc_level.h"

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

/** AABB structure for the tile of the given tile coordinates. */
static struct AABB sc_col_tile_aabb(int x, int y)
{
    struct TilePos tp = { x, y };
    struct WorldPos wp = pos_tile_to_world(tp);
    struct AABB aabb = { wp.x, wp.y, wp.x + sc_tile_w, wp.y + sc_tile_w };
    return aabb;
}

/** Collision of the sides (common to all the character states). */
static void sc_platform_handle_vertical(struct Hunter *hunter, struct CollisionContext *cc)
{
    if ((cc->ltiles[0] == '#' && col_aabb_vline(cc->ltile_aabbs[0], cc->lsline)) ||
        (cc->ltiles[1] == '#' && col_aabb_vline(cc->ltile_aabbs[1], cc->lsline)) ||
        (cc->ltiles[2] == '#' && col_aabb_vline(cc->ltile_aabbs[2], cc->lsline))) {
            hunter->drv.base.stop_x(CMP_DRV(&hunter->drv));
            hunter->ori.current.x =
                cc->ltile_aabbs[0].bx +
                hunter->box_w / 2.0 +
                1.0;
    }
    if ((cc->rtiles[0] == '#' && col_aabb_vline(cc->ltile_aabbs[0], cc->rsline)) ||
        (cc->rtiles[1] == '#' && col_aabb_vline(cc->ltile_aabbs[1], cc->rsline)) ||
        (cc->rtiles[2] == '#' && col_aabb_vline(cc->ltile_aabbs[2], cc->rsline))) {
			hunter->drv.base.stop_x(CMP_DRV(&hunter->drv));
            hunter->ori.current.x =
                cc->rtile_aabbs[0].ax -
                hunter->box_w / 2.0 -
                1.0;
    }
}

/** On the ground specific top/bottom collision. */
static void sc_platform_handle_standing(struct Hunter *hunter, struct CollisionContext *cc)
{
    /* If all of the bottom tiles are non-solid, or don't collide with the
     * bottom box, then start falling.
     */
    if ((cc->btiles[0] != '#' || !col_aabb_aabb(cc->bbox, cc->btile_aabbs[0])) &&
        (cc->btiles[1] != '#' || !col_aabb_aabb(cc->bbox, cc->btile_aabbs[1])) &&
        (cc->btiles[2] != '#' || !col_aabb_aabb(cc->bbox, cc->btile_aabbs[2]))) {
            hunter->standing = false;
    }
}

/** In the air specific top/bottom collision. */
static void sc_platform_handle_midair(
        struct Hunter *hunter,
        struct CollisionContext *cc)
{
    bool collided_up = 
        (cc->utiles[0] == '#' && (col_aabb_vline(cc->utile_aabbs[0], cc->lsline) ||
                                  col_aabb_vline(cc->utile_aabbs[0], cc->rsline))) ||

        (cc->utiles[1] == '#' && (col_aabb_vline(cc->utile_aabbs[1], cc->lsline) ||
                                  col_aabb_vline(cc->utile_aabbs[1], cc->rsline))) ||

        (cc->utiles[2] == '#' && (col_aabb_vline(cc->utile_aabbs[2], cc->lsline) ||
                                  col_aabb_vline(cc->utile_aabbs[2], cc->rsline)));

    bool collided_bottom =
        (cc->btiles[0] == '#' && (col_aabb_vline(cc->btile_aabbs[0], cc->lsline) ||
                                  col_aabb_vline(cc->btile_aabbs[0], cc->rsline))) ||

        (cc->btiles[1] == '#' && (col_aabb_vline(cc->btile_aabbs[1], cc->lsline) ||
                                  col_aabb_vline(cc->btile_aabbs[1], cc->rsline))) ||

        (cc->btiles[2] == '#' && (col_aabb_vline(cc->btile_aabbs[2], cc->lsline) ||
                                  col_aabb_vline(cc->btile_aabbs[2], cc->rsline)));

    if (collided_up) {
		hunter->drv.base.stop_y(CMP_DRV(&hunter->drv));
        hunter->ori.current.y =
            cc->utile_aabbs[0].by +
            hunter->box_h / 2.0 +
            1.0;
    }

    if (collided_bottom) {
            /* TODO: consider getting rid of the move cancelling operation. */
            hunter->standing = true;
			hunter->drv.base.stop_y(CMP_DRV(&hunter->drv));
            hunter->ori.current.y =
                cc->btile_aabbs[0].ay -
                hunter->box_h / 2.0 -
                1.0;
    }
}

/**
 * Analyzes the surrounding of the hunter in the level and records the tiles,
 * their collision structures and the hunter's collision detection shapes in the
 * current context (taking into account the current hunter's position.
 */
static struct CollisionContext sc_platform_analyze(
        struct Hunter *hunter,
        struct Level *lvl)
{
    /* TODO: make this take the hunter's orientation instead of the hunter sturcture. */
    struct PosRot pr = cmp_ori_get(&hunter->ori);
    double w = hunter->box_w;
    double h = hunter->box_h;
    struct TilePos tp = {
		(int)(pr.x / sc_tile_w),
		(int)(pr.y / sc_tile_w)
	};
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
    result.utile_aabbs[0] = sc_col_tile_aabb(tp.x, tp.y - 1);
    result.utile_aabbs[1] = sc_col_tile_aabb(tp.x - 1, tp.y - 1);
    result.utile_aabbs[2] = sc_col_tile_aabb(tp.x + 1, tp.y - 1);
    result.btile_aabbs[0] = sc_col_tile_aabb(tp.x, tp.y + 1);
    result.btile_aabbs[1] = sc_col_tile_aabb(tp.x - 1, tp.y + 1);
    result.btile_aabbs[2] = sc_col_tile_aabb(tp.x + 1, tp.y + 1);
    result.ltile_aabbs[0] = sc_col_tile_aabb(tp.x - 1, tp.y);
    result.ltile_aabbs[1] = sc_col_tile_aabb(tp.x - 1, tp.y - 1);
    result.ltile_aabbs[2] = sc_col_tile_aabb(tp.x - 1, tp.y + 1);
    result.rtile_aabbs[0] = sc_col_tile_aabb(tp.x + 1, tp.y);
    result.rtile_aabbs[1] = sc_col_tile_aabb(tp.x + 1, tp.y - 1);
    result.rtile_aabbs[2] = sc_col_tile_aabb(tp.x + 1, tp.y + 1);

    return result;
}

void sc_platform_draw_debug(void)
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

void sc_platform_collide(struct Hunter *hunter, struct Level *lvl)
{
	cc_last = sc_platform_analyze(hunter, lvl);

	sc_platform_handle_vertical(hunter, &cc_last);
    if (hunter->standing) {
		sc_platform_handle_standing(hunter, &cc_last);
    } else {
		sc_platform_handle_midair(hunter, &cc_last);
    }
}

