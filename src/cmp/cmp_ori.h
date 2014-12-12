/* Copyright (C) 2014 Krzysztof Stachowiak */

#ifndef CMP_ORI_H
#define CMP_ORI_H

struct PosRot {
    double x, y, theta;
};

struct CmpOri {
    struct PosRot current;
    struct PosRot prev;
    int prb_top;
};

struct CmpOri *cmp_ori_create(double x, double y, double theta);
void cmp_ori_free(struct CmpOri *cmp_ori);

void cmp_ori_shift_rotate(
        struct CmpOri *cmp_ori,
        double dx, double dy, double dtheta);

void cmp_ori_cancel_x(struct CmpOri *cmp_ori);
void cmp_ori_cancel_y(struct CmpOri *cmp_ori);

struct PosRot cmp_ori_get(struct CmpOri *cmp_ori);
void cmp_ori_get_shift(struct CmpOri *cmp_ori, double *dx, double *dy);
double cmp_ori_distance(struct CmpOri *a, struct CmpOri *b);

#endif
