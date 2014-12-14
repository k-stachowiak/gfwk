/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <stdlib.h>

#include "diagnostics.h"
#include "memory.h"
#include "cmp_ai.h"

#include "sc_data.h"
#include "sc_graph.h"

enum CmpAiSoulState {
    CMP_AI_SOUL_STATE_IDLE,
    CMP_AI_SOUL_STATE_PANIC,
    CMP_AI_SOUL_STATE_KO,
    CMP_AI_SOUL_STATE_HANGING
};

struct CmpAiSoul {
    struct CmpAi base;
    enum CmpAiSoulState state;
    double think_timer;
    double think_timer_max;
    struct Graph *graph;
};

static void cmp_ai_soul_free(struct CmpAi *cmp_ai)
{
    free_or_die(cmp_ai);
}

static void cmp_ai_soul_update_driver(
        struct CmpDrv *drv,
        struct TilePos *tp,
        struct Graph *lgph)
{
    struct TilePos *points;
    int points_count, i;
    double *coords;

    lgph_random_path(lgph, *tp, &points, &points_count);

    coords = malloc_or_die(sizeof(*coords) * points_count * 2);

    for (i = 0; i < points_count; ++i) {
        struct WorldPos coord_wp = pos_tile_to_world(points[i]);
        coords[2 * i + 0] = coord_wp.x;
        coords[2 * i + 1] = coord_wp.y;
    }

    free_or_die(points);

    cmp_drv_waypoint_reset(drv, coords, points_count);
}

static void cmp_ai_soul_update_idle(
        struct CmpAiSoul *ai,
        struct CmpOri *ori,
        struct CmpDrv *drv,
        double dt)
{
    if ((ai->think_timer -= dt) <= 0.0) {
        ai->think_timer += ai->think_timer_max;
        cmp_ai_soul_update_driver(drv, tp, ai->graph);
    }
}

static void cmp_ai_soul_update_panic(void)
{
}

static void cmp_ai_soul_update_ko(void)
{
}

static void cmp_ai_soul_update_hanging(void)
{
}

static void cmp_ai_soul_update(
        struct CmpAi *this,
        struct CmpOri* ori,
        struct CmpDrv *drv,
        struct CmpAiTacticalStatus *ts,
        double dt)
{
    (void)ts;
    struct CmpAiSoul *derived = (struct CmpAiSoul*)this;
    switch (derived->state) {
    case CMP_AI_SOUL_STATE_IDLE:
        cmp_ai_soul_update_idle();
        break;
    case CMP_AI_SOUL_STATE_PANIC:
        cmp_ai_soul_update_panic(derived, ori, drv, dt);
        break;
    case CMP_AI_SOUL_STATE_KO:
        cmp_ai_soul_update_ko();
        break;
    case CMP_AI_SOUL_STATE_HANGING:
        cmp_ai_soul_update_hanging();
        break;
    }
}

struct CmpAi *cmp_ai_soul_create(struct Graph *graph)
{
    struct CmpAiSoul *result = malloc_or_die(sizeof(*result));

    result->base.free = cmp_ai_soul_free;
    result->base.update = cmp_ai_soul_update;

    result->state = CMP_AI_SOUL_STATE_IDLE;
    result->think_timer = 0.0;
    result->think_timer_max = 2.0;
    result->graph = graph;

    return (struct CmpAi*)result;
}

