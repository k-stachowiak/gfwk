/* Copyright (C) 2014 Krzysztof Stachowiak */

#ifndef CMP_DRV_H
#define CMP_DRV_H

#include <stdbool.h>

struct Vel {
    double vx, vy, vtheta;
};

enum CmpDrvType {
    CMP_DRV_LINEAR,
    CMP_DRV_INPUT_8DIR
};

struct CmpDrv {
    enum CmpDrvType type;
    bool affect_rot;
    union {
        struct Vel lin;
        struct {
            double vel;
            int *inx, *iny;
        } i8d;
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

void cmp_drv_free(struct CmpDrv *cmp_drv);
struct Vel cmp_drv_vel(struct CmpDrv *cmp_drv);

#endif
