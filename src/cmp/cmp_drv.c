/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <stdlib.h>

#include "diagnostics.h"
#include "cmp_drv.h"

struct CmpDrv *cmp_drv_create_linear(
        bool affect_rot,
        double vx, double vy,
        double vtheta)
{
    struct CmpDrv *result = malloc(sizeof(*result));

    if (!result) {
        DIAG_ERROR("Allocation failure.\n");
    }

    result->type = CMP_DRV_LINEAR;
    result->affect_rot = affect_rot;
    result->body.lin.vx = vx;
    result->body.lin.vy = vy;
    result->body.lin.vtheta = vtheta;

    return result;
}

struct CmpDrv *cmp_drv_create_input_8dir(
        bool affect_rot,
        double vel,
        int *inx, int *iny)
{
    struct CmpDrv *result = malloc(sizeof(*result));

    if (!result) {
        DIAG_ERROR("Allocation failure.\n");
    }

    result->type = CMP_DRV_INPUT_8DIR;
    result->affect_rot = affect_rot;
    result->body.i8d.vel = vel;
    result->body.i8d.inx = inx;
    result->body.i8d.iny = iny;

    return result;
}

void cmp_drv_free(struct CmpDrv *cmp_drv)
{
    free(cmp_drv);
}

struct Vel cmp_drv_vel(struct CmpDrv *cmp_drv)
{
    struct Vel result;

    switch (cmp_drv->type) {
    case CMP_DRV_LINEAR:
        result = cmp_drv->body.lin;
        break;
    case CMP_DRV_INPUT_8DIR:
        result.vx = *cmp_drv->body.i8d.inx * cmp_drv->body.i8d.vel;
        result.vy = *cmp_drv->body.i8d.iny * cmp_drv->body.i8d.vel;
        result.vtheta = 0;
        break;
    default:
        DIAG_ERROR("Unhandled driver component type.\n");
    }

    return result;
}

