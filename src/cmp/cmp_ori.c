/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <stdlib.h>
#include <math.h>

#include "diagnostics.h"
#include "memory.h"
#include "cmp_ori.h"

struct CmpOri *cmp_ori_create(double x, double y, double theta)
{
    struct CmpOri *result = malloc_or_die(sizeof(*result));
    struct PosRot init_pr = { x, y, theta };

    result->current = init_pr;
    result->prev = init_pr;

    return result;
}

void cmp_ori_free(struct CmpOri *cmp_ori)
{
    free_or_die(cmp_ori);
}

void cmp_ori_shift_rotate(
        struct CmpOri *cmp_ori,
        double dx, double dy, double dtheta)
{
    cmp_ori->prev = cmp_ori->current;
    cmp_ori->current.x += dx;
    cmp_ori->current.y += dy;
    cmp_ori->current.theta += dtheta;
}

void cmp_ori_cancel_x(struct CmpOri *cmp_ori)
{
    cmp_ori->current.x = cmp_ori->prev.x;
}

void cmp_ori_cancel_y(struct CmpOri *cmp_ori)
{
    cmp_ori->current.x = cmp_ori->prev.x;
}

struct PosRot cmp_ori_get(struct CmpOri *cmp_ori)
{
    return cmp_ori->current;
}

void cmp_ori_get_shift(struct CmpOri *cmp_ori, double *dx, double *dy)
{
    *dx = cmp_ori->current.x - cmp_ori->prev.x;
    *dy = cmp_ori->current.y - cmp_ori->prev.y;
}

double cmp_ori_distance(struct CmpOri *a, struct CmpOri *b)
{
    struct PosRot pra = cmp_ori_get(a);
    struct PosRot prb = cmp_ori_get(b);
    double dx = prb.x - pra.x;
    double dy = prb.y - pra.y;
    return sqrt(dx * dx + dy * dy);
}

