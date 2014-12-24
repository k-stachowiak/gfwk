/* Copyright (C) 2014 Krzysztof Stachowiak */

#ifndef SC_COLLISION_H
#define SC_COLLISION_H

#include <stdbool.h>

struct VLine {
    double x;
    double y1, y2;
};

struct AABB {
    double ax, ay, bx, by;
};

struct Segment {
    double ax, ay, bx, by;
};

struct Circle {
    double x, y, r;
};

void sc_col_convert_circle_cmp(
		struct CmpOri *ori_cmp,
		struct CmpShapeCircle *circle_cmp,
		struct Circle *circle);

void sc_col_convert_segment_cmp(
		struct CmpOri *ori_cmp,
		struct CmpShapeSegment *segment_cmp,
		struct Segment *segment);

bool sc_col_aabb_point(struct AABB aabb, double x, double y);
bool sc_col_aabb_vline(struct AABB aabb, struct VLine vline);
bool sc_col_aabb_aabb(struct AABB lhs, struct AABB rhs);
bool sc_col_segment_circle(struct Segment seg, struct Circle cir);
bool sc_col_circle_circle(struct Circle cir_x, struct Circle cir_y);
bool sc_col_shape_shape(
		struct CmpOri *ori_x, struct CmpShape *shape_x,
		struct CmpOri *ori_y, struct CmpShape *shape_y);

void sc_col_draw_aabb(struct AABB aabb, bool fill, double r, double g, double b);
void sc_col_draw_vline(struct VLine vline, double r, double g, double b);
void sc_col_draw_segment(struct Segment segment, double r, double g, double b);
void sc_col_draw_circle(struct Circle cir, double r, double g, double b);

#endif
