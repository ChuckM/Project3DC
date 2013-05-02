/*
 *
 * ClipTest1.c 
 *
 * $Id: cliptest.c,v 1.1 1999-08-17 18:37:12-07 cmcmanis Exp $
 *
 * This test creates a line and allows it to be rotated in 3-space.
 * It can also be moved and lengthened. It tests the line clipping
 * code 
 *
 */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "g3d.h"


extern int next_char();

static g3d_PNT3 m[4] = {
	{  4.0, -4.0, 4.0 },
	{  4.0,  4.0, -4.0 },
	{ -4.0,  4.0, -4.0 },
	{ -4.0, -4.0, 4.0 },
};

static g3d_PNT3 lines[4] = {
	{ -10, 0, 0 }, { 10, 0, 0 },
	{   0, 5, -5}, { 0, 5, 10 },
};

#define X_AXIS 0
#define Y_AXIS 1
#define Z_AXIS 2

void
roll_line(g3d_LINE *line, int axis) {
	float	ox, oy, nx, ny;

	switch (axis) {
	case X_AXIS:
		ox = line->p1->v.z;
		oy = line->p1->v.y;
		nx = (float) (ox * cos(DTR) + oy * sin(DTR));
		ny = (float) (-ox * sin(DTR) + oy * cos(DTR));
		line->p1->v.z = nx;
		line->p1->v.y = ny;

		ox = line->p2->v.z;
		oy = line->p2->v.y;
		nx = (float) (ox * cos(DTR) + oy * sin(DTR));
		ny = (float) (-ox * sin(DTR) + oy * cos(DTR));
		line->p2->v.z = nx;
		line->p2->v.y = ny;
		line->p1->xid = line->p2->xid = 0;
		return;
	case Y_AXIS:
		ox = line->p1->v.x;
		oy = line->p1->v.z;
		nx = (float) (ox * cos(DTR) + oy * sin(DTR));
		ny = (float) (-ox * sin(DTR) + oy * cos(DTR));
		line->p1->v.x = nx;
		line->p1->v.z = ny;

		ox = line->p2->v.x;
		oy = line->p2->v.z;
		nx = (float) (ox * cos(DTR) + oy * sin(DTR));
		ny = (float) (-ox * sin(DTR) + oy * cos(DTR));
		line->p2->v.x = nx;
		line->p2->v.z = ny;
		line->p1->xid = line->p2->xid = 0;
		return;
	case Z_AXIS:
		ox = line->p1->v.x;
		oy = line->p1->v.y;
		nx = (float) (ox * cos(DTR) + oy * sin(DTR));
		ny = (float) (-ox * sin(DTR) + oy * cos(DTR));
		line->p1->v.x = nx;
		line->p1->v.y = ny;

		ox = line->p2->v.x;
		oy = line->p2->v.y;
		nx = (float) (ox * cos(DTR) + oy * sin(DTR));
		ny = (float) (-ox * sin(DTR) + oy * cos(DTR));
		line->p2->v.x = nx;
		line->p2->v.y = ny;
		line->p1->xid = line->p2->xid = 0;
		return;
	}
	return;
}

void
stretch_line(g3d_LINE *line, float size) {
	float lx, ly, lz;
	lx = line->p2->v.x - line->p1->v.x;
	ly = line->p2->v.y - line->p1->v.y;
	lz = line->p2->v.z - line->p1->v.z;
	line->p2->v.x = (float) (line->p1->v.x + size * lx);
	line->p2->v.y = (float) (line->p1->v.y + size * ly);
	line->p2->v.z = (float) (line->p1->v.z + size * lz);
	line->p1->xid = line->p2->xid = 0;
}

#define INCR 0.55f

g3d_PNT2 dirs[] = {
	{ INCR,     0},
	{ INCR, -INCR},
	{    0, -INCR},
	{-INCR, -INCR},
	{-INCR,     0},
	{-INCR,  INCR},
	{    0,  INCR},
	{ INCR,  INCR},
};

void
move_polygon(g3d_POLY *poly, int dir) {
	g3d_NODE *n;

	poly->cid = 0;
	poly->xid = 0;
	for (n = poly->pn.first; n != NULL; n = n->nxt) {
		g3d_VRTX *v = (g3d_VRTX *)(n->d);
		v->v.x += dirs[dir].u;
		v->v.y += dirs[dir].v;
		v->xid = 0;
	}
}

void
rotate_polygon(g3d_POLY *poly, int axis) {
	g3d_NODE *n;
	float	ox, oy, nx, ny;

	poly->cid = 0;
	poly->xid = 0;
	switch (axis) {
	case X_AXIS:
		for (n = poly->pn.first; n != NULL; n = n->nxt) {
			g3d_VRTX *v = (g3d_VRTX *)(n->d);
			ox = v->v.z;
			oy = v->v.y;
			nx = (float) (ox * cos(DTR) + oy * sin(DTR));
			ny = (float) (-ox * sin(DTR) + oy * cos(DTR));
			v->v.z = nx;
			v->v.y = ny;
		}
		return;
	case Y_AXIS:
		for (n = poly->pn.first; n != NULL; n = n->nxt) {
			g3d_VRTX *v = (g3d_VRTX *)(n->d);
			ox = v->v.x;
			oy = v->v.z;
			nx = (float) (ox * cos(DTR) + oy * sin(DTR));
			ny = (float) (-ox * sin(DTR) + oy * cos(DTR));
			v->v.x = nx;
			v->v.z = ny;
		}
		return;
	case Z_AXIS:
		for (n = poly->pn.first; n != NULL; n = n->nxt) {
			g3d_VRTX *v = (g3d_VRTX *)(n->d);
			ox = v->v.x;
			oy = v->v.y;
			nx = (float) (ox * cos(DTR) + oy * sin(DTR));
			ny = (float) (-ox * sin(DTR) + oy * cos(DTR));
			v->v.x = nx;
			v->v.y = ny;
		}
		return;
	}
	return;
}

void
trace_polygon(g3d_CMRA *cam, g3d_POLY *poly) {
	g3d_LINE line;
	g3d_NODE *n;
	for (n = poly->pn.first; n != NULL; n = n->nxt) {
		g3d_VRTX *a = (g3d_VRTX *)(n->d);
		g3d_VRTX *b = (g3d_VRTX *)((n->nxt) ? (n->nxt->d) : poly->pn.first->d);
		memset((char *)&line, 0, sizeof(line));
		line.p1 = a;
		line.light.x = line.light.y = line.light.z = line.light.w = 1.0;
		line.p2 = b;
		line.color = COLOR_blue;
		g3d_draw_line(cam, 0, &line);
	}
}

void
main(int argc, char *argv[]) {
	g3d_CMRA *cam = g3d_new_camera(60.0f, 0.0f, 0.0f, -30.0f);
	g3d_CMRA *bigCam = g3d_new_camera(60.0f, 0.0f, 0.0f, -30.0f);
	g3d_PNT2 bigPoly[4], littlePoly[4];

	g3d_POLY *poly = NULL;
	g3d_POLY *poly2;
	g3d_LINE *line1, *line2;
	g3d_LIST nodes = { NULL, NULL, G3D_VRTX };
	int using_z = 0;
	float roll = 0;

	g3d_frame_camera(cam, 160, 120, 480, 360);
	g3d_setd_camera(bigCam, cam->d*2);
	bigCam->scalex = bigCam->scalex / 2.0f;
	bigCam->scaley = bigCam->scaley / 2.0f;
	poly = g3d_new_polygon(&COLOR_green, 4, m);
	poly2 = g3d_new_polygon(&COLOR_red, 4, m);
	line1 = g3d_new_line(&COLOR_red, &lines[0], &lines[1]);
	line2 = g3d_new_line(&COLOR_blue, &lines[2], &lines[3]);

    g3d_open();
	while (1) {
		float lx, ly, wx, wy;
		int ndx;
		g3d_NODE *n;

		roll_line(line1, Z_AXIS);
		roll_line(line2, X_AXIS);

		g3d_prepare_frame(&COLOR_lt_gray);
		lx = cam->scx - cam->scalex;
		ly = cam->scy - cam->scaley;
		wx = 2.0f * cam->scalex;
		wy = 2.0f * cam->scaley;
		g3d_draw_axes(cam, G3D_DRAW_CLIPPED, 1.0);
		g3d_clear_region(&COLOR_white, lx, ly, wx, wy);
		for (n = poly->pn.first, ndx = 0; (ndx < 4) && (n != NULL); n = n->nxt) {
			g3d_VRTX v2;
			g3d_xfrm_vrtx(&v2, &(cam->P), (g3d_VRTX *)(n->d));
			littlePoly[ndx].u = v2.v.x;
			littlePoly[ndx].v = v2.v.y;
			g3d_xfrm_vrtx(&v2, &(bigCam->P), (g3d_VRTX *)(n->d));
			bigPoly[ndx].u = v2.v.x;
			bigPoly[ndx].v = v2.v.y;
			ndx++;
		}
		poly->cid = 0;
		poly->xid = 0;
		g3d_draw_polygon(cam, G3D_DRAW_CLIPPED, poly);
		poly->cid = 0;
		poly->xid = 0;
		trace_polygon(cam, poly);
//		g3d_draw_polygon(bigCam, G3D_DRAW_CLIPPED, poly);


//		for (n = poly->pn.first; n != NULL; n = n->nxt) {
//			((g3d_VRTX *)(n->d))->xid = 0;
//		}
		g3d_draw_line(cam, G3D_DRAW_CLIPPED, line1);
		g3d_draw_line(cam, G3D_DRAW_CLIPPED, line2);
		g3d_flip_frame(1);
		switch (next_char()) {
		case 'B':
			/* break point here... */
			lx++;
			break;
		case 'q':
		case 'Q':
			g3d_close();
			return;
		case 'l':
			stretch_line(line1, 1.1f);
			stretch_line(line2, 1.1f);
			break;
		case 'L':
			stretch_line(line1, 0.9f);
			break;
/*
		case 'a':
			g3d_locate_camera(cam, cam->origin.x - 0.5f, cam->origin.y, cam->origin.z);
			break;
		case 'd':
			g3d_locate_camera(cam, cam->origin.x + 0.5f, cam->origin.y, cam->origin.z);
			break;
		case 'w':
			g3d_locate_camera(cam, cam->origin.x, cam->origin.y+0.5f, cam->origin.z);
			break;
		case 'x':
			g3d_locate_camera(cam, cam->origin.x, cam->origin.y - 0.5f, cam->origin.z);
			break;
*/
		case 'f':
			move_polygon(poly, 0);
			move_polygon(poly2, 0);
			break;
		case 'v':
			move_polygon(poly, 1);
			move_polygon(poly2, 1);
			break;
		case 'c':
			move_polygon(poly, 2);
			move_polygon(poly2, 2);
			break;
		case 'x':
			move_polygon(poly, 3);
			move_polygon(poly2, 3);
			break;
		case 's':
			move_polygon(poly, 4);
			move_polygon(poly2, 4);
			break;
		case 'w':
			move_polygon(poly, 5);
			move_polygon(poly2, 5);
			break;
		case 'e':
			move_polygon(poly, 6);
			move_polygon(poly2, 6);
			break;
		case 'r':
			move_polygon(poly, 7);
			move_polygon(poly2, 7);
			break;
		case '[':
			g3d_locate_camera(bigCam, 0, 0, bigCam->origin.z - 0.1f);
			break;
		case ']':
			g3d_locate_camera(bigCam, 0, 0, bigCam->origin.z + 0.1f);
			break;
		case '{':
			g3d_fov_camera(bigCam, bigCam->fov - 0.1f);
			break;
		case '}':
			g3d_fov_camera(bigCam, bigCam->fov + 0.1f);
			break;
		case '1' :
			rotate_polygon(poly, X_AXIS);
			break;
		case '2':
			rotate_polygon(poly, Y_AXIS);
			break;
		case '3':
			rotate_polygon(poly, Z_AXIS);
			break;
		}
	}
	g3d_log("Done rendering.\n");
	g3d_close();
}

