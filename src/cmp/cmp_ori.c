/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <stdlib.h>

#include "diagnostics.h"
#include "cmp_ori.h"

struct CmpOri *cmp_ori_create(double x, double y, double theta)
{
    int i;
    struct CmpOri *result = malloc(sizeof(*result));
    struct PosRot init_pr = { x, y, theta };

    if (!result) {
        DIAG_ERROR("Allocation failure.\n");
    }

    result->prb_top = 0;
    for (i = 0; i < CMP_ORI_BUF_SIZE; ++i) {
        result->pr_buffer[i] = init_pr;
    }

    return result;
}

void cmp_ori_free(struct CmpOri *cmp_ori)
{
    free(cmp_ori);
}

void cmp_ori_shift_rotate(
        struct CmpOri *cmp_ori,
        double dx, double dy, double dtheta)
{
    int prev = cmp_ori->prb_top;
    int curr = (prev + 1) % CMP_ORI_BUF_SIZE;
    cmp_ori->pr_buffer[curr] = cmp_ori->pr_buffer[prev];
    cmp_ori->pr_buffer[curr].x += dx;
    cmp_ori->pr_buffer[curr].y += dy;
    cmp_ori->pr_buffer[curr].theta += dtheta;
    cmp_ori->prb_top = curr;
}

struct PosRot cmp_ori_get(struct CmpOri *cmp_ori)
{
    return cmp_ori->pr_buffer[cmp_ori->prb_top];
}

struct PosRot cmp_ori_get_prev(struct CmpOri *cmp_ori, int delta)
{
    int index = (cmp_ori->prb_top - delta) % CMP_ORI_BUF_SIZE;
    return cmp_ori->pr_buffer[index];
}
