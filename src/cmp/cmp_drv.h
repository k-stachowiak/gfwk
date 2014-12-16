/* Copyright (C) 2014 Krzysztof Stachowiak */

#ifndef CMP_DRV_H
#define CMP_DRV_H

#include <stdbool.h>

typedef void(*CmpDrvCallback)(struct CmpDrv*, void*);

struct Vel {
    double vx, vy, vtheta;
};

struct CmpDrv {
    bool affect_rot;
    void (*free)(struct CmpDrv*);
    void (*update)(struct CmpDrv*, double);
    void (*stop)(struct CmpDrv*, bool, bool);
    void (*stop_x)(struct CmpDrv*);
    void (*stop_y)(struct CmpDrv*);
    struct Vel (*vel)(struct CmpDrv*);
};

struct CmpDrv *cmp_drv_linear_create(bool affect_rot, double vx, double vy, double vtheta);
struct CmpDrv *cmp_drv_i8d_create(bool affect_rot, double vel, int *inx, int *iny); 
struct CmpDrv *cmp_drv_platform_create(int *inx, bool *jump_req, bool *standing);
struct CmpDrv *cmp_drv_ballistic_create(bool affect_rot, double vx, double vy);

struct CmpDrv *cmp_drv_waypoint_create(double velocity);

/* TODO: take care of these hacks. */
void cmp_drv_waypoint_on_end(struct CmpDrv *this, CmpDrvCallback on_end, void *data);
void cmp_drv_waypoint_reset(struct CmpDrv *this, double *points, int points_count);
void cmp_drv_waypoint_points(struct CmpDrv *this, double **points, int *points_count);

#endif
