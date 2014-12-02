/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <stdlib.h>
#include <math.h>

#include "array.h"
#include "diagnostics.h"
#include "cmp_ori.h"
#include "sc_collision.h"
#include "sc_pain.h"

struct PainContext {
    struct {
        struct Segment *data;
        int size, cap;
    } arrs;
    struct Circle soul_cir;
} pc_last;

static struct PainContext pain_analyze(
        struct Arrow *arrows, int arrows_count,
        struct Soul *soul)
{
    int i;
    struct PainContext result;
    struct PosRot soul_pr;

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

    soul_pr = cmp_ori_get(soul->ori);
    result.soul_cir.x = soul_pr.x;
    result.soul_cir.y = soul_pr.y;
    result.soul_cir.r = 25.0;

    return result;
}

void pain_draw_debug(void)
{
    int i;
    for (i = 0; i < pc_last.arrs.size; ++i) {
        col_draw_segment(pc_last.arrs.data[i], 1, 1, 1);
    }
    col_draw_circle(pc_last.soul_cir, 1, 1, 1);
}

void pain_tick(
        struct Arrow *arrows, int arrows_count,
        struct Soul *soul)
{
    int i;
    bool deal_pain;
    struct Circle soul_cir = pc_last.soul_cir;

    if (pc_last.arrs.data) {
        ARRAY_FREE(pc_last.arrs);
    }

    pc_last = pain_analyze(arrows, arrows_count, soul);

    deal_pain = false;
    for (i = 0; i < pc_last.arrs.size; ++i) {
        struct Segment *arr_seg = pc_last.arrs.data + i;
        if (col_segment_circle(*arr_seg, soul_cir))
        {
            deal_pain = true;
            break;
        }
    }

    if (!deal_pain) {
        return;
    }

    DIAG_TRACE("pejn");
}

