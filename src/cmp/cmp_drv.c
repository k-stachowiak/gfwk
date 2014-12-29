/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "diagnostics.h"
#include "memory.h"
#include "cmp_drv.h"
#include "sc_data.h"

/* NEW Impl. */

static void vel_stop(struct Vel *vel, bool x, bool y)
{
	vel->vx -= (!!x) * vel->vx;
	vel->vy -= (!!y) * vel->vy;
}

/* Linear implementation.
 * ======================
 */

static void cmp_drv_linear_deinit(struct CmpDrvLinear *drv)
{
	(void)drv;
}

static void cmp_drv_linear_update(struct CmpDrvLinear *drv, double dt)
{
	(void)drv;
	(void)dt;
}

static void cmp_drv_linear_stop(struct CmpDrvLinear *drv, bool x, bool y)
{
	vel_stop(&drv->vel, x, y);
}

static struct Vel cmp_drv_linear_vel(struct CmpDrvLinear *drv)
{
	return drv->vel;
}

/* Input 8 directions implementation.
 * ==================================
 */

static void cmp_drv_i8d_deinit(struct CmpDrvI8d *drv)
{
	(void)drv;
}

static void cmp_drv_i8d_update(struct CmpDrvI8d *drv, double dt)
{
	(void)drv;
	(void)dt;
}

static void cmp_drv_i8d_stop(struct CmpDrvI8d *drv, bool x, bool y)
{
	(void)drv;
	(void)x;
	(void)y;
}

static struct Vel cmp_drv_i8d_vel(struct CmpDrvI8d *drv)
{
	struct Vel result = {
		*drv->inx * drv->vel,
		*drv->iny * drv->vel
	};
	return result;
}

/* Ballistic implementation.
 * =========================
 */

static void cmp_drv_ballistic_deinit(struct CmpDrvBallistic *drv)
{
	(void)drv;
}

static void cmp_drv_ballistic_update(struct CmpDrvBallistic *drv, double dt)
{
	drv->vel.vy += 10.0 * sc_tile_w * dt;
}

static void cmp_drv_ballistic_stop(struct CmpDrvBallistic *drv, bool x, bool y)
{
	vel_stop(&drv->vel, x, y);
}

static struct Vel cmp_drv_ballistic_vel(struct CmpDrvBallistic *drv)
{
	return drv->vel;
}

/* Platform implementation.
 * ========================
 */

static void cmp_drv_platform_deinit(struct CmpDrvPlatform *drv)
{
	(void)drv;
}

static void cmp_drv_platform_update(struct CmpDrvPlatform *drv, double dt)
{
	if (*drv->standing) {
		if (*drv->jump_req == true) {
			*drv->standing = false;
			drv->vel.vy = -7.0 * sc_tile_w;
		}
	}
	else {
		drv->vel.vy += 10.0 * sc_tile_w * dt;
	}

	drv->vel.vx = *drv->inx * 200.0;

	*drv->jump_req = false;
}

static void cmp_drv_platform_stop(struct CmpDrvPlatform *drv, bool x, bool y)
{
	vel_stop(&drv->vel, x, y);
}

static struct Vel cmp_drv_platform_vel(struct CmpDrvPlatform *drv)
{
	return drv->vel;
}

/* Waypoint implementation.
 * ========================
 */

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

static void cmp_drv_waypoint_deinit(struct CmpDrvWaypoint *drv)
{
	free_or_die(drv->points);
}

static void cmp_drv_waypoint_update(struct CmpDrvWaypoint *drv, double dt)
{
	double x0, y0, x1, y1;
	double dx, dy;
	double rev_sqrt;
	double step_inc;

	cmp_drv_waypoint_local_points(drv, &x0, &y0, &x1, &y1);

	dx = x1 - x0;
	dy = y1 - y0;

	rev_sqrt = 1.0 / sqrt(dx * dx + dy * dy);
	step_inc = drv->velocity * dt * rev_sqrt;

	drv->step_degree += step_inc;
	if (drv->step_degree >= 1.0) {
		drv->step_degree = 0.0;
		++drv->step;
		if (drv->step == (drv->points_count - 1)) {
			drv->on_end((struct CmpDrv*)drv, drv->on_end_data);
		}
	}
}

static void cmp_drv_waypoint_stop(struct CmpDrvWaypoint *drv, bool x, bool y)
{
	(void)x;
	(void)y;
	drv->velocity = 0;
}

struct Vel cmp_drv_waypoint_vel(struct CmpDrvWaypoint *drv)
{
	struct Vel result = { 0.0, 0.0, 0.0 };

	double x0, y0, x1, y1;
	double dx, dy;
	double rev_sqrt;

	cmp_drv_waypoint_local_points(drv, &x0, &y0, &x1, &y1);

	dx = x1 - x0;
	dy = y1 - y0;

	rev_sqrt = 1.0 / sqrt(dx * dx + dy * dy);

	result.vx = dx * rev_sqrt * drv->velocity;
	result.vy = dy * rev_sqrt * drv->velocity;

	return result;
}

/* Proxy implementation.
 * =====================
 */

static void cmp_drv_proxy_deinit(struct CmpDrvProxy *drv)
{
	int i;
	for (i = 0; i < drv->children_count; ++i) {
		cmp_drv_deinit(drv->children + i);
	}
	free_or_die(drv->children);
}

static void cmp_drv_proxy_update(struct CmpDrvProxy *drv, double dt)
{
	struct CmpDrv *child = drv->children + drv->current_child;
	cmp_drv_update(child, dt);
}

static void cmp_drv_proxy_stop(struct CmpDrvProxy *drv, bool x, bool y)
{
	struct CmpDrv *child = drv->children + drv->current_child;
	cmp_drv_stop(child, x, y);
}

static struct Vel cmp_drv_proxy_vel(struct CmpDrvProxy *drv)
{
	struct CmpDrv *child = drv->children + drv->current_child;
	return cmp_drv_vel(child);
}

/* NEW API. */

void cmp_drv_linear_init(
		struct CmpDrv *drv,
		bool affect_rot,
		double vx, double vy,
		double vtheta)
{
	drv->type = CMP_DRV_LINEAR;
	drv->affect_rot = affect_rot;
	drv->body.linear.vel.vx = vx;
	drv->body.linear.vel.vy = vy;
	drv->body.linear.vel.vtheta = vtheta;
}

void cmp_drv_i8d_init(
		struct CmpDrv *drv,
		bool affect_rot,
		double vel,
		int *inx, int *iny)
{
	drv->type = CMP_DRV_I8D;
	drv->type = affect_rot;
	drv->body.i8d.vel = vel;
	drv->body.i8d.inx = inx;
	drv->body.i8d.iny = iny;
}

void cmp_drv_ballistic_init(
		struct CmpDrv *drv,
		bool affect_rot,
		double vx,
		double vy)
{
	drv->type = CMP_DRV_BALLISTIC;
	drv->affect_rot = affect_rot;
	drv->body.ballistic.vel.vx = vx;
	drv->body.ballistic.vel.vy = vy;
	drv->body.ballistic.vel.vtheta = 0.0;
}

void cmp_drv_platform_init(
		struct CmpDrv *drv,
		int *inx,
		bool *jump_req,
		bool *standing)
{
	drv->type = CMP_DRV_PLATFORM;
	drv->affect_rot = false;
	drv->body.platform.vel.vx = 0.0;
	drv->body.platform.vel.vy = 0.0;
	drv->body.platform.vel.vtheta = 0.0;
	drv->body.platform.inx = inx;
	drv->body.platform.jump_req = jump_req;
	drv->body.platform.standing = standing;
}

void cmp_drv_waypoint_init(struct CmpDrv *drv, double velocity)
{
	drv->type = CMP_DRV_WAYPOINT;
	drv->affect_rot = false;
	drv->body.waypoint.points = NULL;
	drv->body.waypoint.points_count = 0;
	drv->body.waypoint.velocity = velocity;
	drv->body.waypoint.step = 0;
	drv->body.waypoint.step_degree = 0.0;
	drv->body.waypoint.on_end_data = NULL;
	drv->body.waypoint.on_end = NULL;
}

void cmp_drv_proxy_init(
		struct CmpDrv *drv,
		struct CmpDrv children[],
		int children_count,
		int init_child)
{
	drv->type = CMP_DRV_PROXY;
	drv->affect_rot = children[init_child].affect_rot;

	drv->body.proxy.children_count = children_count;
	drv->body.proxy.current_child = init_child;

	memcpy(drv->body.proxy.children, children, sizeof(*children) * children_count);
}

void cmp_drv_deinit(struct CmpDrv *drv)
{
	switch (drv->type) {
	case CMP_DRV_LINEAR:
		cmp_drv_linear_deinit((struct CmpDrvLinear*)drv);
		break;
	case CMP_DRV_I8D:
		cmp_drv_i8d_deinit((struct CmpDrvI8d*)drv);
		break;
	case CMP_DRV_BALLISTIC:
		cmp_drv_ballistic_deinit((struct CmpDrvBallistic*)drv);
		break;
	case CMP_DRV_PLATFORM:
		cmp_drv_platform_deinit((struct CmpDrvPlatform*)drv);
		break;
	case CMP_DRV_WAYPOINT:
		cmp_drv_waypoint_deinit((struct CmpDrvWaypoint*)drv);
		break;
	case CMP_DRV_PROXY:
		cmp_drv_proxy_deinit((struct CmpDrvProxy*)drv);
		break;
	}
}

void cmp_drv_update(struct CmpDrv *drv, double dt)
{
	switch (drv->type) {
	case CMP_DRV_LINEAR:
		cmp_drv_linear_update((struct CmpDrvLinear*)drv, dt);
		break;
	case CMP_DRV_I8D:
		cmp_drv_i8d_update((struct CmpDrvI8d*)drv, dt);
		break;
	case CMP_DRV_BALLISTIC:
		cmp_drv_ballistic_update((struct CmpDrvBallistic*)drv, dt);
		break;
	case CMP_DRV_PLATFORM:
		cmp_drv_platform_update((struct CmpDrvPlatform*)drv, dt);
		break;
	case CMP_DRV_WAYPOINT:
		cmp_drv_waypoint_update((struct CmpDrvWaypoint*)drv, dt);
		break;
	case CMP_DRV_PROXY:
		cmp_drv_proxy_update((struct CmpDrvProxy*)drv, dt);
		break;
	}
}

void cmp_drv_stop(struct CmpDrv *drv, bool x, bool y)
{
	switch (drv->type) {
	case CMP_DRV_LINEAR:
		cmp_drv_linear_stop((struct CmpDrvLinear*)drv, x, y);
		break;
	case CMP_DRV_I8D:
		cmp_drv_i8d_stop((struct CmpDrvI8d*)drv, x, y);
		break;
	case CMP_DRV_BALLISTIC:
		cmp_drv_ballistic_stop((struct CmpDrvBallistic*)drv, x, y);
		break;
	case CMP_DRV_PLATFORM:
		cmp_drv_platform_stop((struct CmpDrvPlatform*)drv, x, y);
		break;
	case CMP_DRV_WAYPOINT:
		cmp_drv_waypoint_stop((struct CmpDrvWaypoint*)drv, x, y);
		break;
	case CMP_DRV_PROXY:
		cmp_drv_proxy_stop((struct CmpDrvProxy*)drv, x, y);
		break;
	}
}

void cmp_drv_stop_x(struct CmpDrv *drv)
{
	cmp_drv_stop(drv, true, false);
}

void cmp_drv_stop_y(struct CmpDrv *drv, bool x, bool y)
{
	cmp_drv_stop(drv, false, true);
}

struct Vel cmp_drv_vel(struct CmpDrv *drv)
{
	switch (drv->type) {
	case CMP_DRV_LINEAR:
		return cmp_drv_linear_vel((struct CmpDrvLinear*)drv);
	case CMP_DRV_I8D:
		return cmp_drv_i8d_vel((struct CmpDrvI8d*)drv);
	case CMP_DRV_BALLISTIC:
		return cmp_drv_ballistic_vel((struct CmpDrvBallistic*)drv);
	case CMP_DRV_PLATFORM:
		return cmp_drv_platform_vel((struct CmpDrvPlatform*)drv);
	case CMP_DRV_WAYPOINT:
		return cmp_drv_waypoint_vel((struct CmpDrvWaypoint*)drv);
	case CMP_DRV_PROXY:
		return cmp_drv_proxy_vel((struct CmpDrvProxy*)drv);
	}
	DIAG_ERROR("Should never get here.");
	exit(1);
}

/* Public Waypoint hacks.
 * ======================
 */

void cmp_drv_waypoint_on_end(struct CmpDrv *drv, CmpDrvCallback on_end, void *data)
{
	struct CmpDrvWaypoint *derived = (struct CmpDrvWaypoint*) drv;
	derived->on_end = on_end;
	derived->on_end_data = data;
}

void cmp_drv_waypoint_reset(struct CmpDrv *drv, double *points, int points_count)
{
	struct CmpDrvWaypoint *derived = (struct CmpDrvWaypoint*) drv;
	free_or_die(derived->points);
	derived->points = points;
	derived->points_count = points_count;
	derived->step = 0;
	derived->step_degree = 0;
}

void cmp_drv_waypoint_points(struct CmpDrv *drv, double **points, int *points_count)
{
	struct CmpDrvWaypoint *derived = (struct CmpDrvWaypoint*) drv;
	*points = derived->points;
	*points_count = derived->points_count;
}

/* Public proxy driver hacks.
 * ==========================
 */

int cmp_drv_proxy_get_child(struct CmpDrv *drv)
{
	struct CmpDrvProxy *derived = (struct CmpDrvProxy*)drv;
	return derived->current_child;
}

void cmp_drv_proxy_set_child(struct CmpDrv *drv, int child)
{
	struct CmpDrvProxy *derived = (struct CmpDrvProxy*)drv;
	derived->current_child = child;
}