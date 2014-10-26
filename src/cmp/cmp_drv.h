/* Copyright (C) 2014 Krzysztof Stachowiak */

#ifndef CMP_DRV_H
#define CMP_DRV_H

#include <stdbool.h>

struct Vel {
    double vx, vy, vtheta;
};

struct CmpDrvI8d {
    double vel;
    int *inx, *iny;
};

struct CmpDrvPlat {
    struct Vel vel;
    int *inx;
    bool *jump_req;
    bool *standing;
};

struct CmpDrvWaypoint {
    double *points;
    int points_count;
    bool patrol;
    double velocity;
    int step;
    double step_degree;
    bool flag;
};

enum CmpDrvType {
    CMP_DRV_LINEAR,
    CMP_DRV_INPUT_8DIR,
    CMP_DRV_PLATFORM,
    CMP_DRV_BALLISTIC,
    CMP_DRV_WAYPOINT
};

struct CmpDrv {
    enum CmpDrvType type;
    bool affect_rot;
    union {
        struct Vel lin;
        struct CmpDrvI8d i8d;
        struct CmpDrvPlat plat;
        struct Vel bal;
        struct CmpDrvWaypoint wayp;
    } body;
};

struct CmpDrv *cmp_drv_create_linear(
        bool affect_rot,
        double vx, double vy,
        double vtheta);

struct CmpDrv *cmp_drv_create_input_8dir(
        bool affect_rot,
        double vel,
        int *inx, int *iny);

struct CmpDrv *cmp_drv_create_platform(
        int *inx, bool *jump_req, bool *standing);

struct CmpDrv *cmp_drv_create_ballistic(
        bool affect_rot, double vx, double vy);

struct CmpDrv *cmp_drv_create_waypoint(
        double *points, int points_count,
        bool patrol, double velocity);

void cmp_drv_free(struct CmpDrv *cmp_drv);
void cmp_drv_update(struct CmpDrv *cmp_drv, double dt);
void cmp_drv_stop(struct CmpDrv *cmp_drv, bool x, bool y);
void cmp_drv_stop_x(struct CmpDrv *cmp_drv);
void cmp_drv_stop_y(struct CmpDrv *cmp_drv);
struct Vel cmp_drv_vel(struct CmpDrv *cmp_drv);

#endif
