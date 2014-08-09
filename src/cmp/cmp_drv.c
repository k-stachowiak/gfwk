/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <stdlib.h>

#include "diagnostics.h"
#include "cmp_drv.h"

/* TODO: Handle the tile factor more elegantly. */
static double cmp_drv_tile_factor = 64.0;

static void cmp_drv_update_platform(struct CmpDrvPlat *plat, double dt)
{
    if (*(plat->standing)) {
        if (*plat->jump_req == true) {
            *plat->standing = false;
            plat->vel.vy = -7.0 * cmp_drv_tile_factor;
        }
    } else {
        plat->vel.vy += 10.0 * cmp_drv_tile_factor * dt;
    }

    plat->vel.vx = *plat->inx * 100.0;

    *plat->jump_req = false;
}

void cmp_drv_update_patrol(struct CmpDrvPatr *patr, double dt)
{
    patr->x += patr->v * dt;

    if ((patr->v > 0.0 && patr->x >= patr->x2) ||
        (patr->v <= 0.0 && patr->x <= patr->x1)) {
            *patr->turn_flag = true;
            patr->v *= -1;
    } else {
        *patr->turn_flag = false;
    }
}

struct CmpDrv *cmp_drv_create_linear(
        bool affect_rot,
        double vx, double vy,
        double vtheta)
{
    struct CmpDrv *result = malloc(sizeof(*result));

    if (!result) {
        DIAG_ERROR("Allocation failure.\n");
        exit(1);
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
        exit(1);
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
        exit(1);
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

struct CmpDrv *cmp_drv_create_ballistic(bool affect_rot, double vx, double vy)
{
    struct CmpDrv *result = malloc(sizeof(*result));

    if (!result) {
        DIAG_ERROR("Allocation failure.\n");
        exit(1);
    }

    result->type = CMP_DRV_BALLISTIC;
    result->affect_rot = affect_rot;
    result->body.bal.vx = vx;
    result->body.bal.vy = vy;
    result->body.bal.vtheta = 0.0;

    return result;
}

struct CmpDrv *cmp_drv_create_patrol(
        double x1, double x2,
        double y, double v,
        bool *turn_flag)
{
    struct CmpDrv *result = malloc(sizeof(*result));

    if (!result) {
        DIAG_ERROR("Allocation failure.\n");
        exit(1);
    }

    result->type = CMP_DRV_PATROL;
    result->affect_rot = false;
    result->body.patr.x1 = x1;
    result->body.patr.x2 = x2;
    result->body.patr.y = y;
    result->body.patr.x = x1;
    result->body.patr.v = v;
    result->body.patr.turn_flag = turn_flag;

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
    case CMP_DRV_BALLISTIC:
        cmp_drv->body.bal.vy += 10.0 * cmp_drv_tile_factor * dt;
        break;
    case CMP_DRV_PATROL:
        cmp_drv_update_patrol(&cmp_drv->body.patr, dt);
        break;
    default:
        DIAG_ERROR("Unhandled driver component type.\n");
        exit(1);
    }
}

void cmp_drv_stop(struct CmpDrv *cmp_drv, bool x, bool y)
{
    switch (cmp_drv->type) {
    case CMP_DRV_LINEAR:
        DIAG_WARNING("Attempt to stop linear driver component.\n");
        break;
    case CMP_DRV_INPUT_8DIR:
        DIAG_WARNING("Attempt to stop input 8dir driver component.\n");
        break;
    case CMP_DRV_PLATFORM:
        if (x) {
            cmp_drv->body.plat.vel.vx = 0.0;
        }
        if (y) {
            cmp_drv->body.plat.vel.vy = 0.0;
        }
        break;
    case CMP_DRV_BALLISTIC:
        if (x) {
            cmp_drv->body.bal.vx = 0.0;
        }
        if (y) {
            cmp_drv->body.bal.vy = 0.0;
        }
        break;
    case CMP_DRV_PATROL:
        if (x) {
            cmp_drv->body.patr.v = 0.0;
        }
        if (y) {
            DIAG_WARNING("Attempt to y stop patrol driver component.\n");
        }
        break;
    default:
        DIAG_ERROR("Unhandled driver component type.\n");
        exit(1);
    }
}

void cmp_drv_stop_x(struct CmpDrv *cmp_drv)
{
    cmp_drv_stop(cmp_drv, true, false);
}

void cmp_drv_stop_y(struct CmpDrv *cmp_drv)
{
    cmp_drv_stop(cmp_drv, false, true);
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
        result.vtheta = 0.0;
        break;
    case CMP_DRV_PLATFORM:
        result = cmp_drv->body.plat.vel;
        break;
    case CMP_DRV_BALLISTIC:
        result = cmp_drv->body.bal;
        break;
    case CMP_DRV_PATROL:
        result.vx = cmp_drv->body.patr.v;
        result.vy = 0.0;
        break;
    default:
        DIAG_ERROR("Unhandled driver component type.\n");
        exit(1);
    }

    return result;
}

