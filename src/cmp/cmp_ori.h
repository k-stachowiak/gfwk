/* Copyright (C) 2014 Krzysztof Stachowiak */

#ifndef CMP_ORI_H
#define CMP_ORI_H

#define CMP_ORI_BUF_SIZE 10

struct PosRot {
    double x, y, theta;
};

struct CmpOri {
    struct PosRot pr_buffer[CMP_ORI_BUF_SIZE];
    int prb_top;
};

struct CmpOri *cmp_ori_create(double x, double y, double theta);
void cmp_ori_free(struct CmpOri *cmp_ori);

void cmp_ori_shift_rotate(
        struct CmpOri *cmp_ori,
        double dx, double dy, double dtheta);

struct PosRot cmp_ori_get(struct CmpOri *cmp_ori);
struct PosRot cmp_ori_get_prev(struct CmpOri *cmp_ori, int delta);

#endif
