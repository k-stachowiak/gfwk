/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <stdlib.h>

#include "diagnostics.h"
#include "cmp_ori.h"

static float rsqrt(float number)
{
    long i;
    float x2, y;
    float threehalfs = 1.5f;
    x2 = number * 0.5f;
    y = number;
    i = *(long*) &y;
    i = 0x5f3759df - (i >> 1);
    y = *(float*)&i;
    y = y * (threehalfs - (x2 * y * y));
    y = y * (threehalfs - (x2 * y * y));
    return y;
}

struct CmpOri *cmp_ori_create(double x, double y, double theta)
{
    struct CmpOri *result = malloc(sizeof(*result));
    struct PosRot init_pr = { x, y, theta };

    if (!result) {
        DIAG_ERROR("Allocation failure.");
        exit(1);
    }

    result->current = init_pr;
    result->prev = init_pr;

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

double cmp_ori_distance(struct CmpOri *a, struct CmpOri *b)
{
    struct PosRot pra = cmp_ori_get(a);
    struct PosRot prb = cmp_ori_get(b);
    double dx = prb.x - pra.x;
    double dy = prb.y - pra.y;
    return 1.0 / rsqrt(dx * dx + dy * dy);
}

