/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <stdlib.h>

#include "mymath.h"
#include "diagnostics.h"
#include "cmp_drv.h"
#include "sc_data.h"

static void cmp_drv_update_platform(struct CmpDrvPlat *plat, double dt)
{
    if (*(plat->standing)) {
        if (*plat->jump_req == true) {
            *plat->standing = false;
            plat->vel.vy = -7.0 * sc_tile_w;
        }
    } else {
        plat->vel.vy += 10.0 * sc_tile_w * dt;
    }

    plat->vel.vx = *plat->inx * 100.0;

    *plat->jump_req = false;
}

static void cmp_drv_waypoint_local_points(
        struct CmpDrvWaypoint *wayp,
        double *x0, double *y0,
        double *x1, double *y1)
{
    if (!wayp->patrol || wayp->flag) {
       *x0 = wayp->points[2 * wayp->step + 0];
       *y0 = wayp->points[2 * wayp->step + 1];
       *x1 = wayp->points[2 * wayp->step + 2];
       *y1 = wayp->points[2 * wayp->step + 3];
   } else {
       *x0 = wayp->points[2 * wayp->step + 0];
       *y0 = wayp->points[2 * wayp->step + 1];
       *x1 = wayp->points[2 * wayp->step - 2];
       *y1 = wayp->points[2 * wayp->step - 1];
   }
}

static void cmp_drv_waypoint_step(struct CmpDrvWaypoint *wayp)
{
    wayp->step_degree = 0.0;

    if (wayp->patrol) {
        if (wayp->flag) {
            ++wayp->step;
            if(wayp->step >= (wayp->points_count - 1)) {
                wayp->flag = false;
            }
        } else {
            --wayp->step;
            if(wayp->step <= 0) {
                wayp->flag = true;
            }
        }

    } else {
        ++wayp->step;
        if(wayp->step >= (wayp->points_count - 1)) {
            wayp->flag = true;
        }
    }
}

void cmp_drv_update_waypoint(struct CmpDrvWaypoint *wayp, double dt)
{
    double x0, y0, x1, y1;
    double dx, dy;
    double step_len, step_inc;

    if (!wayp->patrol && wayp->flag) {
        return;
    }

    cmp_drv_waypoint_local_points(wayp, &x0, &y0, &x1, &y1);

    dx = x1 - x0;
    dy = y1 - y0;

    step_len = 1.0 / rsqrt(dx * dx + dy * dy);
    step_inc = (wayp->velocity * dt) / step_len;

    wayp->step_degree += step_inc;
    if (wayp->step_degree > 1.0) {
        cmp_drv_waypoint_step(wayp);
    }
}

struct Vel cmp_drv_vel_waypoint(struct CmpDrvWaypoint *wayp)
{
    struct Vel result = { 0.0, 0.0, 0.0 };

    double x0, y0, x1, y1;
    double dx, dy;
    double rev_sqrt;

    if (!wayp->patrol && wayp->flag) {
        return result;
    }

    cmp_drv_waypoint_local_points(wayp, &x0, &y0, &x1, &y1);

    dx = x1 - x0;
    dy = y1 - y0;

    rev_sqrt = rsqrt(dx * dx + dy * dy);

    result.vx = dx * rev_sqrt * wayp->velocity;
    result.vy = dy * rev_sqrt * wayp->velocity;

    return result;
}

struct CmpDrv *cmp_drv_create_linear(
        bool affect_rot,
        double vx, double vy,
        double vtheta)
{
    struct CmpDrv *result = malloc(sizeof(*result));

    if (!result) {
        DIAG_ERROR("Allocation failure.");
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
        DIAG_ERROR("Allocation failure.");
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
        DIAG_ERROR("Allocation failure.");
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
        DIAG_ERROR("Allocation failure.");
        exit(1);
    }

    result->type = CMP_DRV_BALLISTIC;
    result->affect_rot = affect_rot;
    result->body.bal.vx = vx;
    result->body.bal.vy = vy;
    result->body.bal.vtheta = 0.0;

    return result;
}

struct CmpDrv *cmp_drv_create_waypoint(
        double *points, int points_count,
        bool patrol, double velocity)
{
    struct CmpDrv *result = malloc(sizeof(*result));

    if (!result) {
        DIAG_ERROR("Allocation failure.");
        exit(1);
    }

    result->type = CMP_DRV_WAYPOINT;
    result->affect_rot = false;
    result->body.wayp.points = points;
    result->body.wayp.points_count = points_count;
    result->body.wayp.patrol = patrol;
    result->body.wayp.velocity = velocity;
    result->body.wayp.step = 0;
    result->body.wayp.step_degree = 0.0;
    result->body.wayp.flag = patrol;

    return result;
}

void cmp_drv_free(struct CmpDrv *cmp_drv)
{
    if (cmp_drv->type == CMP_DRV_WAYPOINT) {
        free(cmp_drv->body.wayp.points);
    }
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
        cmp_drv->body.bal.vy += 10.0 * sc_tile_w * dt;
        break;
    case CMP_DRV_WAYPOINT:
        cmp_drv_update_waypoint(&cmp_drv->body.wayp, dt);
        break;
    default:
        DIAG_ERROR("Unhandled driver component type.");
        exit(1);
    }
}

void cmp_drv_stop(struct CmpDrv *cmp_drv, bool x, bool y)
{
    switch (cmp_drv->type) {
    case CMP_DRV_LINEAR:
        DIAG_WARNING("Attempt to stop linear driver component.");
        break;
    case CMP_DRV_INPUT_8DIR:
        DIAG_WARNING("Attempt to stop input 8dir driver component.");
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
    case CMP_DRV_WAYPOINT:
        if (x) {
            cmp_drv->body.wayp.velocity = 0.0;
        }
        if (y) {
            DIAG_WARNING("Attempt to y stop patrol driver component.");
        }
        break;
    default:
        DIAG_ERROR("Unhandled driver component type.");
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
    struct Vel result = { 0.0, 0.0, 0.0 };

    switch (cmp_drv->type) {
    case CMP_DRV_LINEAR:
        result = cmp_drv->body.lin;
        break;
    case CMP_DRV_INPUT_8DIR:
        result.vx = *cmp_drv->body.i8d.inx * cmp_drv->body.i8d.vel;
        result.vy = *cmp_drv->body.i8d.iny * cmp_drv->body.i8d.vel;
        break;
    case CMP_DRV_PLATFORM:
        result = cmp_drv->body.plat.vel;
        break;
    case CMP_DRV_BALLISTIC:
        result = cmp_drv->body.bal;
        break;
    case CMP_DRV_WAYPOINT:
        result = cmp_drv_vel_waypoint(&cmp_drv->body.wayp);
        break;
    default:
        DIAG_ERROR("Unhandled driver component type.");
        exit(1);
    }

    return result;
}

