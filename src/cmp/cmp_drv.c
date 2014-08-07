/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <stdlib.h>

#include "diagnostics.h"
#include "cmp_drv.h"

static void cmp_drv_update_platform(struct CmpDrvPlat *plat, double dt)
{
    if (*(plat->standing)) {
        if (*(plat->jump_req) == true) {
            *(plat->standing) = false;
            plat->vel.vy = -7.0 * 64.0;
        }
    } else {
        /* TODO: Handle the tile factor more elegantly here (64). */
        plat->vel.vy += 10.0 * 64.0 * dt;
    }

    plat->vel.vx = *(plat->inx) * 100.0;

    *(plat->jump_req) = false;
}

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

struct CmpDrv *cmp_drv_create_platform(
        int *inx, bool *jump_req, bool *standing)
{
    struct CmpDrv *result = malloc(sizeof(*result));

    if (!result) {
        DIAG_ERROR("Allocation failure.\n");
    }

    result->type = CMP_DRV_PLATFORM;
    result->affect_rot = false;
    result->body.plat.vel.vx = 0.0;
    result->body.plat.vel.vy = 0.0;
    result->body.plat.vel.vtheta = 0.0;
    result->body.plat.inx = inx;
    result->body.plat.jump_req = jump_req;
    result->body.plat.standing = standing;

    return result;
}

void cmp_drv_free(struct CmpDrv *cmp_drv)
{
    free(cmp_drv);
}

void cmp_drv_update(struct CmpDrv *cmp_drv, double dt)
{
    switch (cmp_drv->type) {
    case CMP_DRV_LINEAR:
    case CMP_DRV_INPUT_8DIR:
        break;
    case CMP_DRV_PLATFORM:
        cmp_drv_update_platform(&cmp_drv->body.plat, dt);
        break;
    default:
        DIAG_ERROR("Unhandled driver component type.\n");
    }
}

void cmp_drv_stop(struct CmpDrv *cmp_drv)
{
    switch (cmp_drv->type) {
    case CMP_DRV_LINEAR:
        DIAG_WARNING("Attempt to stop linear driver component.\n");
        break;
    case CMP_DRV_INPUT_8DIR:
        DIAG_WARNING("Attempt to stop input 8dir driver component.\n");
        break;
    case CMP_DRV_PLATFORM:
        cmp_drv->body.plat.vel.vx = 0.0;
        cmp_drv->body.plat.vel.vy = 0.0;
        cmp_drv->body.plat.vel.vtheta = 0.0;
        break;
    default:
        DIAG_ERROR("Unhandled driver component type.\n");
    }
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
    case CMP_DRV_PLATFORM:
        result = cmp_drv->body.plat.vel;
        break;
    default:
        DIAG_ERROR("Unhandled driver component type.\n");
    }

    return result;
}

