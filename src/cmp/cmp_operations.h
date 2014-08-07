/* Copyright (C) 2014 Krzysztof Stachowiak */

#ifndef CMP_OPERATIONS_H
#define CMP_OPERATIONS_H

void cmp_draw(struct CmpOri *ori, struct CmpAppr *appr, double vsx, double vsy);
void cmp_drive(struct CmpOri *ori, struct CmpDrv *drv, double dt);

#endif
