/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <stdlib.h>

#include "diagnostics.h"
#include "cmp_ai.h"

#include "sc_data.h"
#include "sc_graph.h"

/* Soul AI component implementation.
 * =================================
 */

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
};

static void cmp_ai_soul_free(struct CmpAi *cmp_ai)
{
    free(cmp_ai);
}

static void cmp_ai_soul_update(
        struct CmpAi *this,
        struct CmpDrv *drv,
        struct CmpAiTacticalStatus *ts,
        double dt)
{
    (void)drv;
    (void)ts;

    struct CmpAiSoul *derived = (struct CmpAiSoul*)this;

    derived->think_timer -= dt;
    if (derived->think_timer <= 0.0) {
        derived->think_timer += derived->think_timer_max;
    }
}

static void cmp_ai_soul_update_driver(
        struct CmpAi *this,
        struct CmpDrv *drv,
        struct TilePos *soul_tp,
        struct Graph *lgph)
{
    struct TilePos *points;
    int points_count, i;
    double *coords;

    (void)this;

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

    cmp_drv_waypoint_reset(drv, coords, points_count);
}

struct CmpAi *cmp_ai_soul_create(void)
{
    struct CmpAiSoul *result = malloc(sizeof(*result));

    if (!result) {
        DIAG_ERROR("Allocation failure.");
        exit(1);
    }

    result->base.free = cmp_ai_soul_free;
    result->base.update = cmp_ai_soul_update;
    result->base.update_driver = cmp_ai_soul_update_driver;

    result->state = CMP_AI_SOUL_STATE_IDLE;
    result->think_timer = 0.0;
    result->think_timer_max = 2.0;

    return (struct CmpAi*)result;
}

