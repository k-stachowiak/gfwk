/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <stdlib.h>
#include <math.h>

#include "array.h"
#include "cmp_ori.h"
#include "sc_collision.h"
#include "sc_pain.h"

struct PainContext {
    struct {
        struct Segment *data;
        int size, cap;
    } arrs;
} pc_last;

static struct PainContext pain_analyze(
        struct Arrow *arrows, int arrows_count,
        struct Soul *soul)
{
    int i;
    struct PainContext result;

    result.arrs.data = NULL;
    result.arrs.size = 0;
    result.arrs.cap = 0;

    for (i = 0; i < arrows_count; ++i) {
        struct PosRot pr = cmp_ori_get(arrows[i].ori);
        struct Segment seg = {
            pr.x, pr.y,
            pr.x + 25 * cos(pr.theta),
            pr.y + 25 * sin(pr.theta)
        };
        ARRAY_APPEND(result.arrs, seg);
    }

    return result;
}

void pain_draw_debug(void)
{
    int i;
    for (i = 0; i < pc_last.arrs.size; ++i) {
        col_draw_segment(pc_last.arrs.data[i], 1, 1, 1);
    }
}

void pain_tick(
        struct Arrow *arrows, int arrows_count,
        struct Soul *soul)
{
    if (pc_last.arrs.data) {
        ARRAY_FREE(pc_last.arrs);
    }
    pc_last = pain_analyze(arrows, arrows_count, soul);
}

