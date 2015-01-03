/* Copyright (C) 2014 Krzysztof Stachowiak */

#ifndef CMP_DRV_H
#define CMP_DRV_H

#include <stdbool.h>

/* Common data types.
 * ==================
 */

typedef void(*CmpDrvCallback)(struct CmpDrv*, void*);

struct Vel {
    double vx, vy, vtheta;
};

/* Linear driver.
 * ==============
 */

struct CmpDrvLinear {
	struct Vel vel;
};

/* 8 directions input driver.
 * ==========================
 */

struct CmpDrvI8d {
	double vel;
	int *inx, *iny;
};

/* Ballistic driver.
 * =================
 */

struct CmpDrvBallistic {
	struct Vel vel;
};

/* Platform driver.
 * ================
 */

struct CmpDrvPlatform
{
	struct Vel vel;
	int *inx;
	bool *jump_req;
	bool *standing;
};

/* Waypoint driver.
 * ================
 */

struct CmpDrvWaypoint {
	double *points;
	int points_count;
	double velocity;
	int step;
	double step_degree;
	void *on_end_data;
	CmpDrvCallback on_end;
};

/* Proxy driver.
 * =============
 */

struct CmpDrvProxy {
	struct CmpDrv *children;
	int children_count;
	int current_child;
};

/* Root structure.
 * ===============
 */

enum CmpDrvType {
	CMP_DRV_LINEAR,
	CMP_DRV_I8D,
	CMP_DRV_BALLISTIC,
	CMP_DRV_PLATFORM,
	CMP_DRV_WAYPOINT
};

struct CmpDrv {
	union {
		struct CmpDrvLinear linear;
		struct CmpDrvI8d i8d;
		struct CmpDrvBallistic ballistic;
		struct CmpDrvPlatform platform;
		struct CmpDrvWaypoint waypoint;
	} body;
	enum CmpDrvType type;
	bool affect_rot;
};

void cmp_drv_linear_init(
		struct CmpDrv *drv,
		bool affect_rot,
		double vx, double vy,
		double vtheta);

void cmp_drv_i8d_init(
		struct CmpDrv *drv,
		bool affect_rot,
		double vel,
		int *inx, int *iny);

void cmp_drv_ballistic_init(
		struct CmpDrv *drv,
		bool affect_rot,
		double vx,
		double vy);

void cmp_drv_platform_init(
		struct CmpDrv *drv,
		int *inx,
		bool *jump_req,
		bool *standing);

void cmp_drv_waypoint_init(struct CmpDrv *drv, double velocity);

void cmp_drv_deinit(struct CmpDrv *drv);

void cmp_drv_update(struct CmpDrv *drv, double dt);
void cmp_drv_stop(struct CmpDrv *drv, bool x, bool y);
void cmp_drv_stop_x(struct CmpDrv*);
void cmp_drv_stop_y(struct CmpDrv*);
struct Vel cmp_drv_vel(struct CmpDrv*);

void cmp_drv_waypoint_on_end(struct CmpDrv *drv, CmpDrvCallback on_end, void *data);
void cmp_drv_waypoint_reset(struct CmpDrv *drv, double *points, int points_count);
void cmp_drv_waypoint_points(struct CmpDrv *drv, double **points, int *points_count);

#endif
