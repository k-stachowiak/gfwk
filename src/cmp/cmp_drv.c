/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <stdlib.h>
#include <math.h>

#include "diagnostics.h"
#include "memory.h"
#include "cmp_drv.h"
#include "sc_data.h"

/* Base class implementation.
 * ==========================
 */

static void cmp_drv_base_free(struct CmpDrv *this)
{
    free_or_die(this);
}

static void cmp_drv_base_stop_x(struct CmpDrv *this)
{
    this->stop(this, true, false);
}

static void cmp_drv_base_stop_y(struct CmpDrv *this)
{
    this->stop(this, false, true);
}

static void cmp_drv_base_init(struct CmpDrv *this, double affect_rot)
{
    this->affect_rot = affect_rot;
    this->free = cmp_drv_base_free;
    this->stop_x = cmp_drv_base_stop_x;
    this->stop_y = cmp_drv_base_stop_y;
}

/* Common class implementation.
 * =============================
 */

static void cmp_drv_common_update(struct CmpDrv *this, double dt)
{
    (void)this;
    (void)dt;
}

static void cmp_drv_common_stop(struct CmpDrv *this, bool x, bool y)
{
    (void)this;
    (void)x;
    (void)y;
    DIAG_WARNING("Attempt to stop incompatible driver component.");
}


/* Linear driver implementation.
 * =============================
 */

struct CmpDrvLinear {
    struct CmpDrv base;
    struct Vel vel;
};

static struct Vel cmp_drv_linear_vel(struct CmpDrv *this)
{
    struct CmpDrvLinear *derived = (struct CmpDrvLinear*)this;
    return derived->vel;
}

struct CmpDrv *cmp_drv_linear_create(
        bool affect_rot,
        double vx, double vy,
        double vtheta)
{
    struct CmpDrvLinear *result = malloc_or_die(sizeof(*result));

    cmp_drv_base_init((struct CmpDrv*)result, affect_rot);

    result->base.update = cmp_drv_common_update;
    result->base.stop = cmp_drv_common_stop;
    result->base.vel = cmp_drv_linear_vel;

    result->vel.vx = vx;
    result->vel.vy = vy;
    result->vel.vtheta = vtheta;

    return (struct CmpDrv*)result;
}

/* 8 directions input driver implementation.
 * =========================================
 */

struct CmpDrvI8d {
    struct CmpDrv base;
    double vel;
    int *inx, *iny;
};

static struct Vel cmp_drv_i8d_vel(struct CmpDrv *this)
{
    struct Vel result = { 0, };
    struct CmpDrvI8d *derived = (struct CmpDrvI8d*)this;

    result.vx = *derived->inx * derived->vel;
    result.vy = *derived->iny * derived->vel;

    return result;
}

struct CmpDrv *cmp_drv_i8d_create(
        bool affect_rot,
        double vel,
        int *inx, int *iny)
{
    struct CmpDrvI8d *result = malloc_or_die(sizeof(*result));

    cmp_drv_base_init((struct CmpDrv*)result, affect_rot);

    result->base.update = cmp_drv_common_update;
    result->base.stop = cmp_drv_common_stop;
    result->base.vel = cmp_drv_i8d_vel;

    result->vel = vel;
    result->inx = inx;
    result->iny = iny;

    return (struct CmpDrv*)result;
}

/* Ballistic driver implementation.
 * ================================
 */

struct CmpDrvBallistic {
    struct CmpDrv base;
    struct Vel vel;
};

static void cmp_drv_ballistic_update(struct CmpDrv *this, double dt)
{
    struct CmpDrvBallistic *derived = (struct CmpDrvBallistic*)this;
    derived->vel.vy += 10.0 * sc_tile_w * dt;
}

static void cmp_drv_ballistic_stop(struct CmpDrv *this, bool x, bool y)
{
    struct CmpDrvBallistic *derived = (struct CmpDrvBallistic*)this;
    derived->vel.vx -= (!!x) * derived->vel.vx;
    derived->vel.vy -= (!!y) * derived->vel.vy;
}

static struct Vel cmp_drv_ballistic_vel(struct CmpDrv *this)
{
    struct CmpDrvBallistic *derived = (struct CmpDrvBallistic*)this;
    return derived->vel;
}

struct CmpDrv *cmp_drv_ballistic_create(
        bool affect_rot,
        double vx, double vy)
{
    struct CmpDrvBallistic *result = malloc_or_die(sizeof(*result));

    cmp_drv_base_init((struct CmpDrv*)result, affect_rot);

    result->base.update = cmp_drv_ballistic_update;
    result->base.stop = cmp_drv_ballistic_stop;
    result->base.vel = cmp_drv_ballistic_vel;

    result->vel.vx = vx;
    result->vel.vy = vy;
    result->vel.vtheta = 0.0;

    return (struct CmpDrv*)result;
}

/* Platform driver implementation.
 * ===============================
 */

struct CmpDrvPlatform
{
    struct CmpDrv base;
    struct Vel vel;
    int *inx;
    bool *jump_req;
    bool *standing;
};

static void cmp_drv_platform_update(struct CmpDrv *this, double dt)
{
    struct CmpDrvPlatform *derived = (struct CmpDrvPlatform*)this;

    if (*derived->standing) {
        if (*derived->jump_req == true) {
            *derived->standing = false;
            derived->vel.vy = -7.0 * sc_tile_w;
        }
    } else {
        derived->vel.vy += 10.0 * sc_tile_w * dt;
    }

    derived->vel.vx = *derived->inx * 200.0;

    *derived->jump_req = false;
}

static void cmp_drv_platform_stop(struct CmpDrv *this, bool x, bool y)
{
    struct CmpDrvPlatform *derived = (struct CmpDrvPlatform*)this;
    derived->vel.vx -= (!!x) * derived->vel.vx;
    derived->vel.vy -= (!!y) * derived->vel.vy;
}

static struct Vel cmp_drv_platform_vel(struct CmpDrv *this)
{
    struct CmpDrvPlatform *derived = (struct CmpDrvPlatform*)this;
    return derived->vel;
}

struct CmpDrv *cmp_drv_platform_create(
        int *inx, bool *jump_req, bool *standing)
{
    struct CmpDrvPlatform *result = malloc_or_die(sizeof(*result));

    cmp_drv_base_init((struct CmpDrv*)result, false);

    result->base.update = cmp_drv_platform_update;
    result->base.stop = cmp_drv_platform_stop;
    result->base.vel = cmp_drv_platform_vel;

    result->vel.vx = 0.0;
    result->vel.vy = 0.0;
    result->vel.vtheta = 0.0;
    result->inx = inx;
    result->jump_req = jump_req;
    result->standing = standing;

    return (struct CmpDrv*)result;
}

/* Waypoint driver implementation.
 * ===============================
 */

struct CmpDrvWaypoint {

    struct CmpDrv base;

    double *points;
    int points_count;

    double velocity;
    int step;
    double step_degree;

	void *on_end_data;
	CmpDrvCallback on_end;
};

static void cmp_drv_waypoint_free(struct CmpDrv *this)
{
    struct CmpDrvWaypoint *derived = (struct CmpDrvWaypoint*)this;
    free_or_die(derived->points);
    cmp_drv_base_free(this);
}

static void cmp_drv_waypoint_local_points(
        struct CmpDrvWaypoint *wayp,
        double *x0, double *y0,
        double *x1, double *y1)
{
    *x0 = wayp->points[2 * wayp->step + 0];
    *y0 = wayp->points[2 * wayp->step + 1];
    *x1 = wayp->points[2 * wayp->step + 2];
    *y1 = wayp->points[2 * wayp->step + 3];
}

static void cmp_drv_waypoint_update(struct CmpDrv *this, double dt)
{
    struct CmpDrvWaypoint *derived = (struct CmpDrvWaypoint*)this;

    double x0, y0, x1, y1;
    double dx, dy;
    double step_len, step_inc;

    cmp_drv_waypoint_local_points(derived, &x0, &y0, &x1, &y1);

    dx = x1 - x0;
    dy = y1 - y0;

    step_len = sqrt(dx * dx + dy * dy);
    step_inc = (derived->velocity * dt) / step_len;

    derived->step_degree += step_inc;
    if (derived->step_degree > 1.0) {
        derived->step_degree -= 1.0;
		++derived->step;
		if (derived->step == (derived->points_count - 1)) { /* WHY -1 ? ;( */
			derived->on_end(this, derived->on_end_data);
		}
    }
}

static void cmp_drv_waypoint_stop(struct CmpDrv *this, bool x, bool y)
{
    struct CmpDrvWaypoint *derived = (struct CmpDrvWaypoint*)this;
    (void)x;
    (void)y;
    derived->velocity = 0;
}

struct Vel cmp_drv_waypoint_vel(struct CmpDrv *this)
{
    struct CmpDrvWaypoint *derived = (struct CmpDrvWaypoint*)this;
    struct Vel result = { 0.0, 0.0, 0.0 };

    double x0, y0, x1, y1;
    double dx, dy;
    double rev_sqrt;

    cmp_drv_waypoint_local_points(derived, &x0, &y0, &x1, &y1);

    dx = x1 - x0;
    dy = y1 - y0;

    rev_sqrt = 1.0 / sqrt(dx * dx + dy * dy);

    result.vx = dx * rev_sqrt * derived->velocity;
    result.vy = dy * rev_sqrt * derived->velocity;

    return result;
}

struct CmpDrv *cmp_drv_waypoint_create(double velocity)
{
    struct CmpDrvWaypoint *result = malloc_or_die(sizeof(*result));

    cmp_drv_base_init((struct CmpDrv*)result, false);

    result->base.free = cmp_drv_waypoint_free;
    result->base.update = cmp_drv_waypoint_update;
    result->base.stop = cmp_drv_waypoint_stop;
    result->base.vel = cmp_drv_waypoint_vel;

    result->points = NULL;
    result->points_count = 0;
    result->velocity = velocity;
    result->step = 0;
    result->step_degree = 0.0;
	result->on_end_data = NULL;
	result->on_end = NULL;

    return (struct CmpDrv*)result;
}

void cmp_drv_waypoint_on_end(struct CmpDrv *this, CmpDrvCallback on_end, void *data)
{
	struct CmpDrvWaypoint *derived = (struct CmpDrvWaypoint*)this;
	derived->on_end = on_end;
	derived->on_end_data = data;
}

void cmp_drv_waypoint_reset(struct CmpDrv *this, double *points, int points_count)
{
    struct CmpDrvWaypoint *derived = (struct CmpDrvWaypoint*)this;
    free_or_die(derived->points);
    derived->points = points;
    derived->points_count = points_count;
	derived->step = 0;
	derived->step_degree = 0;
}

void cmp_drv_waypoint_points(struct CmpDrv *this, double **points, int *points_count)
{
    struct CmpDrvWaypoint *derived = (struct CmpDrvWaypoint*)this;
    *points = derived->points;
    *points_count = derived->points_count;
}
