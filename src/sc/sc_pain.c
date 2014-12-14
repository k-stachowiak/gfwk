/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <stdlib.h>
#include <math.h>

#include "array.h"
#include "diagnostics.h"
#include "cmp_ori.h"
#include "cmp_operations.h"
#include "sc_collision.h"
#include "sc_pain.h"

struct PainContext {
    struct {
        struct Segment *data;
        int size, cap;
    } arrow_segs;
    struct Circle soul_cir;
} pc_last;

static void pain_reset_components(
        struct Arrow *arrows, int arrows_count,
        struct Soul *soul)
{
    int i;
    cmp_pain_reset(soul->pain);
    for (i = 0; i < arrows_count; ++i) {
        cmp_pain_reset(arrows[i].pain);
    }
}

static void pain_tick_arrows(
        struct Arrow *arrows, int arrows_count,
        struct Soul *soul, struct Circle *soul_cir)
{
    int i;
    for (i = 0; i < arrows_count; ++i) {

        struct PosRot arrow_pr = cmp_ori_get(arrows[i].ori);
        struct Segment arrow_seg;

        arrow_seg.ax = arrow_pr.x;
        arrow_seg.ay = arrow_pr.y;
        arrow_seg.bx = arrow_pr.x + 25 * cos(arrow_pr.theta);
        arrow_seg.by = arrow_pr.y + 25 * sin(arrow_pr.theta);

        ARRAY_APPEND(pc_last.arrow_segs, arrow_seg);

        if (col_segment_circle(arrow_seg, *soul_cir)) {
            cmp_deal_pain(soul->pain, arrows[i].pain);
        }
    }
}

void pain_draw_debug(void)
{
    int i;
    for (i = 0; i < pc_last.arrow_segs.size; ++i) {
        col_draw_segment(pc_last.arrow_segs.data[i], 1, 1, 1);
    }
    col_draw_circle(pc_last.soul_cir, 1, 1, 1);
}

void pain_tick(
        struct Arrow *arrows, int arrows_count,
        struct Soul *soul)
{
    struct PosRot soul_pr;
    struct Circle soul_cir;

    ARRAY_FREE(pc_last.arrow_segs);
    pain_reset_components(arrows, arrows_count, soul);
    
    soul_pr = cmp_ori_get(soul->ori);
    soul_cir.x = soul_pr.x;
    soul_cir.y = soul_pr.y;
    soul_cir.r = 25.0;

    pc_last.soul_cir = soul_cir;

    pain_tick_arrows(arrows, arrows_count, soul, &soul_cir);
}

