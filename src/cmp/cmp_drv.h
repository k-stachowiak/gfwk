/* Copyright (C) 2014 Krzysztof Stachowiak */

#ifndef CMP_DRV_H
#define CMP_DRV_H

#include <stdbool.h>

/* Common data types.
 * ==================
 */

struct CmpDrv;

typedef void(*CmpDrvCallback)(struct CmpDrv*, void*);

struct Vel {
    double vx, vy, vtheta;
};

/* Base class.
 * ===========
 */

struct CmpDrv {
    bool affect_rot;
    void (*free)(struct CmpDrv*);
    void (*update)(struct CmpDrv*, double);
    void (*stop)(struct CmpDrv*, bool, bool);
    void (*stop_x)(struct CmpDrv*);
    void (*stop_y)(struct CmpDrv*);
    struct Vel (*vel)(struct CmpDrv*);
};

#define CMP_DRV(T) (struct CmpDrv*)(T)

/* Linear driver.
 * ==============
 */

struct CmpDrvLinear {
	struct CmpDrv base;
	struct Vel vel;
};

void cmp_drv_linear_init(
		struct CmpDrvLinear *drv,
		bool affect_rot,
		double vx, double vy,
		double vtheta);

void cmp_drv_linear_deinit(struct CmpDrvLinear* drv);

struct CmpDrv *cmp_drv_linear_create(bool affect_rot, double vx, double vy, double vtheta);

/* 8 directions input driver.
 * ==========================
 */

struct CmpDrvI8d {
	struct CmpDrv base;
	double vel;
	int *inx, *iny;
};

void cmp_drv_i8d_init(
		struct CmpDrvI8d *drv,
		bool affect_rot,
		double vel,
		int *inx, int *iny);

void cmp_drv_i8d_deinit(struct CmpDrvI8d *drv);

struct CmpDrv *cmp_drv_i8d_create(bool affect_rot, double vel, int *inx, int *iny);

/* Ballistic driver.
 * =================
 */

struct CmpDrvBallistic {
	struct CmpDrv base;
	struct Vel vel;
};

void cmp_drv_ballistic_init(
		struct CmpDrvBallistic *drv,
		bool affect_rot,
		double vx,
		double vy);

void cmp_drv_ballistic_deinit(struct CmpDrvBallistic *drv);

struct CmpDrv *cmp_drv_ballistic_create(bool affect_rot, double vx, double vy);

/* Platform driver.
 * ================
 */

struct CmpDrvPlatform
{
	struct CmpDrv base;
	struct Vel vel;
	int *inx;
	bool *jump_req;
	bool *standing;
};

void cmp_drv_platform_init(
		struct CmpDrvPlatform *drv,
		int *inx,
		bool *jump_req,
		bool *standing);

void cmp_drv_platform_deinit(struct CmpDrvPlatform *drv);

struct CmpDrv *cmp_drv_platform_create(int *inx, bool *jump_req, bool *standing);

/* Waypoint driver.
 * ================
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

void cmp_drv_waypoint_init(struct CmpDrvWaypoint *drv, double velocity);
void cmp_drv_waypoint_deinit(struct CmpDrvWaypoint *drv);
struct CmpDrv *cmp_drv_waypoint_create(double velocity);

void cmp_drv_waypoint_on_end(struct CmpDrv *this, CmpDrvCallback on_end, void *data);
void cmp_drv_waypoint_reset(struct CmpDrv *this, double *points, int points_count);
void cmp_drv_waypoint_points(struct CmpDrv *this, double **points, int *points_count);

#endif
