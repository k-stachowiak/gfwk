/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <stdlib.h>

#include "diagnostics.h"
#include "cmp_ai.h"

#include "sc_data.h"
#include "sc_graph.h"

static void cmp_ai_update_soul(
        struct CmpAiSoul *ai,
        struct CmpDrvWaypoint *wp,
        struct CmpAiTacticalStatus *ts,
        double dt)
{
    (void)wp;
    (void)ts;

    ai->think_timer -= dt;
    if (ai->think_timer <= 0.0) {
        ai->think_timer += ai->think_timer_max;
    }
}

struct CmpAi *cmp_ai_soul_create(enum CmpAiSoulState init_state)
{
    struct CmpAi *result = malloc(sizeof(*result));

    if (!result) {
        DIAG_ERROR("Allocation failure.");
        exit(1);
    }

    result->type = CMP_AI_SOUL;
    result->body.soul.state = init_state;
    result->body.soul.think_timer = 0.0;
    result->body.soul.think_timer_max = 2.0;

    return result;
}

void cmp_ai_soul_update_driver(
        struct CmpAiSoul *ai,
        struct CmpDrvWaypoint *wp,
        struct TilePos *soul_tp,
        struct Graph *lgph)
{
    struct TilePos *points;
    int points_count, i;
    double *coords;

    lgph_random_path(lgph, *soul_tp, &points, &points_count);

    coords = malloc(sizeof(*coords) * points_count * 2);
    if (!coords) {
        DIAG_ERROR("Allocation failure.\n");
        exit(1);
    }

    for (i = 0; i < points_count; ++i) {
        struct WorldPos coord_wp = pos_tile_to_world(points[i]);
        coords[2 * i + 0] = coord_wp.x;
        coords[2 * i + 1] = coord_wp.y;
    }

    free(points);

    cmp_drv_waypoint_reset(wp, coords, points_count);
}


void cmp_ai_free(struct CmpAi *cmp_ai)
{
    free(cmp_ai);
}

void cmp_ai_update(
        struct CmpAi *ai,
        struct CmpDrv *drv,
        struct CmpAiTacticalStatus *ts,
        double dt)
{
    switch (ai->type) {
    case CMP_AI_SOUL:
        assert(drv->type == CMP_DRV_WAYPOINT);
        cmp_ai_update_soul(&ai->body.soul, &drv->body.wayp, ts, dt);
        break;
    }
}

