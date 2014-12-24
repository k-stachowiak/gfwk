/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <allegro5/allegro_primitives.h>

#include "diagnostics.h"

#include "cmp_ori.h"
#include "cmp_drv.h"
#include "cmp_shape.h"

#include "sc_collision.h"
#include "sc_data.h"

static double sc_dot(double x1, double y1, double x2, double y2)
{
    return x1 * x2 + y1 * y2;
}

static void sc_point_to_screen(double xi, double yi, double *xo, double *yo)
{
    struct WorldPos wp = { xi, yi };
    struct ScreenPos sp = pos_world_to_screen(wp);
    *xo = sp.x;
    *yo = sp.y;
}

static void sc_vline_to_screen(
		struct VLine vline,
		double *x, double *y1, double *y2)
{
	sc_point_to_screen(vline.x, vline.y1, x, y1);
	sc_point_to_screen(vline.x, vline.y2, x, y2);
}

static void sc_aabb_to_screen(
		struct AABB aabb,
		double *x1, double *y1,
		double *x2, double *y2)
{
	sc_point_to_screen(aabb.ax, aabb.ay, x1, y1);
	sc_point_to_screen(aabb.bx, aabb.by, x2, y2);
}

static void sc_segment_to_screen(
		struct Segment segment,
		double *x1, double *y1,
		double *x2, double *y2)
{
	sc_point_to_screen(segment.ax, segment.ay, x1, y1);
	sc_point_to_screen(segment.bx, segment.by, x2, y2);
}

void sc_col_convert_circle_cmp(
		struct CmpOri *ori_cmp,
		struct CmpShapeCircle *circle_cmp,
		struct Circle *circle)
{
	struct PosRot pr = cmp_ori_get(ori_cmp);
	circle->x = pr.x;
	circle->y = pr.y;
	circle->r = circle_cmp->r;
}

void sc_col_convert_segment_cmp(
		struct CmpOri *ori_cmp,
		struct CmpShapeSegment *segment_cmp,
		struct Segment *segment)
{
	struct PosRot pr = cmp_ori_get(ori_cmp);
	double sine = sin(pr.theta), cosine = cos(pr.theta);

	/* P0 is implicitly (0,0), so we only apply the translation. */
	double x0 = pr.x;
	double y0 = pr.y;

	/* P1 is stored in the component. We apply rotation and then translation. */
	double x1 = (segment_cmp->x1 * cosine - segment_cmp->y1 * sine) + pr.x;
	double y1 = (segment_cmp->x1 * sine + segment_cmp->y1 * cosine) + pr.y;

	segment->ax = x0;
	segment->ay = y0;
	segment->bx = x1;
	segment->by = y1;
}

bool sc_col_aabb_point(struct AABB aabb, double x, double y)
{
    return x < aabb.bx && x > aabb.ax && y < aabb.by && y > aabb.ay;
}

bool sc_col_aabb_vline(struct AABB aabb, struct VLine vline)
{
    return
        vline.x < aabb.bx && vline.x > aabb.ax &&
        vline.y1 < aabb.by && vline.y2 > aabb.ay;
}

bool sc_col_aabb_aabb(struct AABB lhs, struct AABB rhs)
{
    return
        lhs.ax < rhs.bx && lhs.bx > rhs.ax &&
        lhs.ay < rhs.by && lhs.by > rhs.ay;
}

bool sc_col_segment_circle(struct Segment seg, struct Circle cir)
{
    double dx = seg.bx - seg.ax;
    double dy = seg.by - seg.ay;
    double fx = seg.ax - cir.x;
    double fy = seg.ay - cir.y;

	double a = sc_dot(dx, dy, dx, dy);
	double b = 2 * sc_dot(fx, fy, dx, dy);
	double c = sc_dot(fx, fy, fx, fy) - cir.r * cir.r;

    double delta = b * b - 4 * a * c;

    if (delta < 0) {
        return false;
    }

    double sqr_delta = sqrt(delta);

    double t1 = (-b - sqr_delta) / (2 * a);
    double t2 = (-b + sqr_delta) / (2 * a);

    if (t1 >= 0 && t1 <= 1) {
        return true;
    }

    if (t2 >= 0 && t2 <= 1) {
        return true;
    }

    return false;
}

bool sc_col_circle_circle(struct Circle cir_x, struct Circle cir_y)
{
	/* Note: since the cartesian distance is monotonous, we may skip the square root. */
	double sum_r_2 = (cir_x.r + cir_y.r) * (cir_x.r + cir_y.r);
	double dx = cir_y.x - cir_x.x;
	double dy = cir_y.y - cir_x.y;
	double dist_2 = dx * dx + dy * dy;
	return dist_2 < sum_r_2;
}

static bool sc_col_segment_segment(struct Segment seg_x, struct Segment seg_y)
{
	DIAG_ERROR("Not implemented yet.\n");
	exit(1);
	return false;
}

static bool sc_col_circle_shape(
		struct Circle cir_x,
		struct CmpOri *ori_y,
		struct CmpShape *shape_y)
{
	switch (shape_y->type) {
	case CMP_SHAPE_CIRCLE:
		{
			struct Circle cir_y;
			sc_col_convert_circle_cmp(ori_y, &shape_y->body.circle, &cir_y);
			return sc_col_circle_circle(cir_x, cir_y);
		}
	case CMP_SHAPE_SEGMENT:
		{
			struct Segment seg_y;
			sc_col_convert_segment_cmp(ori_y, &shape_y->body.segment, &seg_y);
			return sc_col_segment_circle(seg_y, cir_x);
		}
	}

	DIAG_ERROR("It is impossible to get here.");
	exit(1);
	return false;
}

static bool sc_col_segment_shape(
		struct Segment seg_x,
		struct CmpOri *ori_y,
		struct CmpShape *shape_y)
{
	switch (shape_y->type) {
	case CMP_SHAPE_CIRCLE:
		{
			struct Circle cir_y;
			sc_col_convert_circle_cmp(ori_y, &shape_y->body.circle, &cir_y);
			return sc_col_segment_circle(seg_x, cir_y);
		}
	case CMP_SHAPE_SEGMENT:
		{
			struct Segment seg_y;
			sc_col_convert_segment_cmp(ori_y, &shape_y->body.segment, &seg_y);
			return sc_col_segment_segment(seg_x, seg_y);
		}
	}

	DIAG_ERROR("It is impossible to get here.");
	exit(1);
	return false;
}

bool sc_col_shape_shape(
		struct CmpOri *ori_x, struct CmpShape *shape_x,
		struct CmpOri *ori_y, struct CmpShape *shape_y)
{
	switch (shape_x->type) {
	case CMP_SHAPE_CIRCLE:
		{
			struct Circle cir_x;
			sc_col_convert_circle_cmp(ori_x, &shape_x->body.circle, &cir_x);
			return sc_col_circle_shape(cir_x, ori_y, shape_y);
		}
	case CMP_SHAPE_SEGMENT:
		{
			struct Segment seg_x;
			sc_col_convert_segment_cmp(ori_x, &shape_x->body.segment, &seg_x);
			return sc_col_segment_shape(seg_x, ori_y, shape_y);
		}
	}

	DIAG_ERROR("It is impossible to get here.");
	exit(1);
	return false;
}

void sc_col_draw_aabb(struct AABB aabb, bool fill, double r, double g, double b)
{
    double x1, y1, x2, y2;
	sc_aabb_to_screen(aabb, &x1, &y1, &x2, &y2);
    if (fill) {
        al_draw_filled_rectangle(x1, y1, x2, y2, al_map_rgb_f(r, g, b));
    } else {
        al_draw_rectangle(x1, y1, x2, y2, al_map_rgb_f(r, g, b), 1.0);
    }
}

void sc_col_draw_vline(struct VLine vline, double r, double g, double b)
{
    double x, y1, y2;
	sc_vline_to_screen(vline, &x, &y1, &y2);
    al_draw_line(x, y1, x, y2, al_map_rgb_f(r, g, b), 1.0);
}

void sc_col_draw_segment(struct Segment segment, double r, double g, double b)
{
    double x1, x2, y1, y2;
	sc_segment_to_screen(segment, &x1, &x2, &y1, &y2);
    al_draw_line(x1, x2, y1, y2, al_map_rgb_f(r, g, b), 1.0);
}

void sc_col_draw_circle(struct Circle cir, double r, double g, double b)
{
    double x, y;
	sc_point_to_screen(cir.x, cir.y, &x, &y);
    al_draw_circle(x, y, cir.r, al_map_rgb_f(r, g, b), 1.0);
}

