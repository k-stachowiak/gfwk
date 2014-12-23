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

static struct Vel cmp_drv_linear_vel(struct CmpDrv *this)
{
    struct CmpDrvLinear *derived = (struct CmpDrvLinear*)this;
    return derived->vel;
}

void cmp_drv_linear_init(
		struct CmpDrvLinear *drv,
		bool affect_rot,
		double vx, double vy,
		double vtheta)
{
	cmp_drv_base_init((struct CmpDrv*)drv, affect_rot);

	drv->base.update = cmp_drv_common_update;
	drv->base.stop = cmp_drv_common_stop;
	drv->base.vel = cmp_drv_linear_vel;

	drv->vel.vx = vx;
	drv->vel.vy = vy;
	drv->vel.vtheta = vtheta;
}

void cmp_drv_linear_deinit(struct CmpDrvLinear* drv)
{
	(void)drv;
}

struct CmpDrv *cmp_drv_linear_create(
        bool affect_rot,
        double vx, double vy,
        double vtheta)
{
    struct CmpDrvLinear *result = malloc_or_die(sizeof(*result));
	cmp_drv_linear_init(result, affect_rot, vx, vy, vtheta);
    return (struct CmpDrv*)result;
}

/* 8 directions input driver implementation.
 * =========================================
 */

static struct Vel cmp_drv_i8d_vel(struct CmpDrv *this)
{
    struct Vel result = { 0, };
    struct CmpDrvI8d *derived = (struct CmpDrvI8d*)this;

    result.vx = *derived->inx * derived->vel;
    result.vy = *derived->iny * derived->vel;

    return result;
}

void cmp_drv_i8d_init(
		struct CmpDrvI8d *drv,
		bool affect_rot,
		double vel,
		int *inx, int *iny)
{
	cmp_drv_base_init((struct CmpDrv*)drv      , affect_rot);

	drv->base.update = cmp_drv_common_update;
	drv->base.stop = cmp_drv_common_stop;
	drv->base.vel = cmp_drv_i8d_vel;

	drv->vel = vel;
	drv->inx = inx;
	drv->iny = iny;
}

void cmp_drv_i8d_deinit(struct CmpDrvI8d *drv)
{
	(void)drv;
}

struct CmpDrv *cmp_drv_i8d_create(
        bool affect_rot,
        double vel,
        int *inx, int *iny)
{
    struct CmpDrvI8d *result = malloc_or_die(sizeof(*result));
	cmp_drv_i8d_init(result, affect_rot, vel, inx, iny);
    return (struct CmpDrv*)result;
}

/* Ballistic driver implementation.
 * ================================
 */

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

void cmp_drv_ballistic_init(
		struct CmpDrvBallistic *drv,
		bool affect_rot,
		double vx,
		double vy)
{
	cmp_drv_base_init((struct CmpDrv*)drv, affect_rot);

	drv->base.update = cmp_drv_ballistic_update;
	drv->base.stop = cmp_drv_ballistic_stop;
	drv->base.vel = cmp_drv_ballistic_vel;

	drv->vel.vx = vx;
	drv->vel.vy = vy;
	drv->vel.vtheta = 0.0;
}

void cmp_drv_ballistic_deinit(struct CmpDrvBallistic *drv)
{
	(void)drv;
}

struct CmpDrv *cmp_drv_ballistic_create(
        bool affect_rot,
        double vx, double vy)
{
    struct CmpDrvBallistic *result = malloc_or_die(sizeof(*result));
	cmp_drv_ballistic_init(result, affect_rot, vx, vy);
    return (struct CmpDrv*)result;
}

/* Platform driver implementation.
 * ===============================
 */

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

void cmp_drv_platform_init(
		struct CmpDrvPlatform *drv,
		int *inx,
		bool *jump_req,
		bool *standing)
{
	cmp_drv_base_init((struct CmpDrv*)drv, false);

	drv->base.update = cmp_drv_platform_update;
	drv->base.stop = cmp_drv_platform_stop;
	drv->base.vel = cmp_drv_platform_vel;

	drv->vel.vx = 0.0;
	drv->vel.vy = 0.0;
	drv->vel.vtheta = 0.0;
	drv->inx = inx;
	drv->jump_req = jump_req;
	drv->standing = standing;
}

void cmp_drv_platform_deinit(struct CmpDrvPlatform *drv)
{
	(void)drv;
}

struct CmpDrv *cmp_drv_platform_create(
        int *inx, bool *jump_req, bool *standing)
{
    struct CmpDrvPlatform *result = malloc_or_die(sizeof(*result));
	cmp_drv_platform_init(result, inx, jump_req, standing);
    return (struct CmpDrv*)result;
}

/* Waypoint driver implementation.
 * ===============================
 */

static void cmp_drv_waypoint_free(struct CmpDrv *this)
{
    struct CmpDrvWaypoint *derived = (struct CmpDrvWaypoint*)this;
	cmp_drv_waypoint_deinit(derived);
	cmp_drv_base_free(this);
	/* TODO: Get rid of this common base magic... */
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
	double rev_sqrt;
    double step_inc;
	 
    cmp_drv_waypoint_local_points(derived, &x0, &y0, &x1, &y1);

    dx = x1 - x0;
    dy = y1 - y0;

    rev_sqrt = 1.0 / sqrt(dx * dx + dy * dy);
    step_inc = derived->velocity * dt * rev_sqrt;

    derived->step_degree += step_inc;
    if (derived->step_degree >= 1.0) {
        derived->step_degree = 0.0;
		++derived->step;
		if (derived->step == (derived->points_count - 1)) {
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

void cmp_drv_waypoint_init(struct CmpDrvWaypoint *drv, double velocity)
{
	cmp_drv_base_init((struct CmpDrv*)drv, false);

	drv->base.free = cmp_drv_waypoint_free;
	drv->base.update = cmp_drv_waypoint_update;
	drv->base.stop = cmp_drv_waypoint_stop;
	drv->base.vel = cmp_drv_waypoint_vel;

	drv->points = NULL;
	drv->points_count = 0;
	drv->velocity = velocity;
	drv->step = 0;
	drv->step_degree = 0.0;
	drv->on_end_data = NULL;
	drv->on_end = NULL;
}

void cmp_drv_waypoint_deinit(struct CmpDrvWaypoint *drv)
{
	free_or_die(drv->points);
}

struct CmpDrv *cmp_drv_waypoint_create(double velocity)
{
    struct CmpDrvWaypoint *result = malloc_or_die(sizeof(*result));
	cmp_drv_waypoint_init(result, velocity);
    return (struct CmpDrv*)result;
}

void cmp_drv_waypoint_on_end(struct CmpDrvWaypoint *this, CmpDrvCallback on_end, void *data)
{
	this->on_end = on_end;
	this->on_end_data = data;
}

void cmp_drv_waypoint_reset(struct CmpDrvWaypoint *this, double *points, int points_count)
{
	free_or_die(this->points);
    this->points = points;
    this->points_count = points_count;
	this->step = 0;
	this->step_degree = 0;
}

void cmp_drv_waypoint_points(struct CmpDrvWaypoint *this, double **points, int *points_count)
{
	*points = this->points;
	*points_count = this->points_count;
}


/* Proxy driver implementation.
* =============================
*/

static void cmp_drv_proxy_update(struct CmpDrv* this, double dt)
{
	struct CmpDrvProxy *derived = (struct CmpDrvProxy*)this;
	struct CmpDrv *child = derived->children[derived->current_child];
	child->update(child, dt);
}

static void cmp_drv_proxy_stop(struct CmpDrv* this, bool x, bool y)
{
	struct CmpDrvProxy *derived = (struct CmpDrvProxy*)this;
	struct CmpDrv *child = derived->children[derived->current_child];
	child->stop(child, x, y);
}

static struct Vel cmp_drv_proxy_vel(struct CmpDrv* this)
{
	struct CmpDrvProxy *derived = (struct CmpDrvProxy*)this;
	struct CmpDrv *child = derived->children[derived->current_child];
	return child->vel(child);
}

void cmp_drv_proxy_init(
		struct CmpDrvProxy *drv,
		struct CmpDrv *children[],
		int children_count,
		int init_child)
{
	cmp_drv_base_init((struct CmpDrv*)drv, children[init_child]->affect_rot);

	drv->base.update = cmp_drv_proxy_update;
	drv->base.stop = cmp_drv_proxy_stop;
	drv->base.vel = cmp_drv_proxy_vel;

	drv->children = children;
	drv->children_count = children_count;
	drv->current_child = init_child;
}

void cmp_drv_proxy_deinit(struct CmpDrvProxy *drv)
{
	(void)drv;
}

struct CmpDrv *cmp_drv_proxy_create(
		struct CmpDrv *children[],
		int children_count,
		int init_child)

{
	struct CmpDrvProxy *result = malloc_or_die(sizeof(*result));
	cmp_drv_proxy_init(result, children, children_count, init_child);
	return (struct CmpDrv*)result;
}

int cmp_drv_proxy_get_child(struct CmpDrv *this)
{
	struct CmpDrvProxy *derived = (struct CmpDrvProxy*)this;
	return derived->current_child;
}

void cmp_drv_proxy_set_child(struct CmpDrv *this, int child)
{
	struct CmpDrvProxy *derived = (struct CmpDrvProxy*)this;
	derived->current_child = child;
}