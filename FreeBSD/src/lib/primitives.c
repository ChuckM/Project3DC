/*
 * primitive.c - Drawing primitive management functions for P3DC
 * $Id: primitives.c,v 1.4 1999-09-11 23:31:02-07 cmcmanis Exp $
 *
 * This module implements the drawing primitive management functions for p3dc
 *
 * Change Log:
 *	$Log: primitives.c,v $
 *	Revision 1.4  1999/09/12 06:31:02  cmcmanis
 *	Rewrote clipping api (again?)
 *
 *	Revision 1.2  1999/09/02 03:22:45  cmcmanis
 *	The great LIST rewrite, lists are much cleaner now
 *
 *	Revision 1.1  1999/08/18 01:33:54  cmcmanis
 *	Initial revision
 *
 *	Revision 1.1  1999/08/18 01:06:18  cmcmanis
 *	Initial revision
 *
 *	Revision 1.6  1999/08/14 23:18:39  cmcmanis
 *	This version now includes a new model structure that allows
 *	for dynamic changing of colors and textures in the model
 *	after it has been loaded.
 *
 *	Revision 1.5  1999/07/25 09:11:55  cmcmanis
 *	The great renaming from g3d/G3D -> p3dc/P3DC
 *
 *	Revision 1.4  1998/11/16 03:40:47  cmcmanis
 *	Updated clipping
 *
 *	Revision 1.4  1998/11/08 05:21:19  cmcmanis
 *	Outdated (renamed to 'H:\P3DC-RCS\src\lib\primitives.c,v').
 *
 *	Revision 1.3  1998/11/05 20:23:45  cmcmanis
 *	Checkin prior to moving into a better source tree structure
 *
 *	Revision 1.2  1998/10/10 07:00:42  cmcmanis
 *	Model parsing is written (not fully debugged) and the texture structures
 *	are closer to their final form. By the next check in we should have a
 *	few simple models up and running with textured models to follow.
 *
 *	Revision 1.1  1998/09/20 08:11:07  cmcmanis
 *	Initial revision
 *
 * Copyright statement:
 *
 * All of the documentation and software included in the Generic
 * 3D library is copyrighted by Chuck McManis.
 *
 *   Copyright (c) 1997, 1998 Chuck McManis, all rights
 *   reserved. 
 * 
 * Redistribution and use in source and binary forms, with or
 * without modification, is permitted provided that the terms
 * specified in the file license.txt are met.
 * 
 * THIS SOFTWARE IS PROVIDED BY CHUCK MCMANIS "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL CHUCK MCMANIS BE LIABLE FOR AND
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "p3dc.h"

p3dc_CLR *
p3dc_new_color(p3dc_CLR *color) {
	p3dc_CLR *res;
	res = p3dc_new_type(P3DC_CLR);
	if (res)
		*res = *color;
	return res;
}

/*
 * Create a new p3dc_LINE primitive using a color and two points
 * in 3D space.
 */
p3dc_LINE *
p3dc_new_line(p3dc_CLR *color, p3dc_PNT3 *a, p3dc_PNT3 *b) {
	p3dc_LINE *res;

	res = p3dc_new_type(P3DC_LINE);
	if (res == NULL)
		return NULL;

	res->p1 = p3dc_new_vrtx(a->x, a->y, a->z);
	res->p2 = p3dc_new_vrtx(b->x, b->y, b->z);
	res->color = p3dc_new_color(color);
	res->light.x = res->light.y = res->light.z = res->light.w = 1.0;
	return res;
}

/*
 * Free a Line for re-use.
 */
void
p3dc_free_line(p3dc_LINE *lin) {
	if (lin->p1)
		p3dc_free_vrtx(&(lin->p1));
	if (lin->p2)
		p3dc_free_vrtx(&(lin->p2));
	p3dc_free_type(lin);
}

/*
 * Allocate a new p3dc_POINT primitive using a color and one point
 * in 3D space.
 */
p3dc_POINT *
p3dc_new_point(p3dc_CLR *color, p3dc_PNT3 *a) {
	p3dc_POINT *res;

	res = p3dc_new_type(P3DC_POINT);
	if (res == NULL)
		return NULL;

	res->p0 = p3dc_new_vrtx(a->x, a->y, a->z);
	res->color = p3dc_new_color(color);
	res->lid = 0;
	res->light.x = res->light.y = res->light.z = res->light.w = 1.0;
	return res;
}

/*
 * Free a p3dc_POINT structure.
 */
void
p3dc_free_point(p3dc_POINT *p) {
	if (p->p0)
		p3dc_free_vrtx(&(p->p0));
	p3dc_free_type(p);
}

static void
initialize_polygon(p3dc_POLY *res, p3dc_CLR *color, int nVerts, p3dc_PNT3 vrtx[]) {
	p3dc_NODE *n;
	p3dc_PNT3 v1, v2;
	int i;

	res->color = p3dc_new_color(color);
	res->xid = 0;
	res->lid = 0;
	res->light.x = res->light.y = res->light.z = res->light.w = 1.0;
	res->cid = 0;
	p3dc_init_list(&(res->pn), P3DC_VRTX, P3DC_LIST_LINKED);
	res->clip = NULL;

	for (i = 0; i < nVerts; i++) {
		n = p3dc_new_node(p3dc_new_vrtx(vrtx[i].x, vrtx[i].y, vrtx[i].z), 
												NULL, P3DC_NODE_FREEPAYLOAD);
		p3dc_add_node(&(res->pn), n, P3DC_LIST_TAIL);
	}

	/* Compute the plane normal using the first three vertices */
	p3dc_diff_pnt3(&(vrtx[0]), &(vrtx[1]), &v1);
	p3dc_diff_pnt3(&(vrtx[0]), &(vrtx[2]), &v2);
	p3dc_cross_pnt3(&v2, &v1, &(res->norm));
	p3dc_normalize_pnt3(&(res->norm));
}

/*
 * Create a new p3dc_POLY structure using a color and an array
 * of points in 3D space. The points must be co-planar, and the
 * polygon must be convex! Remember these are PRIMITIVES...
 */
p3dc_POLY *
p3dc_new_polygon(p3dc_CLR *color, int nVerts, p3dc_PNT3 vrtx[]) {
	p3dc_POLY *res;

	if (nVerts < 3)
		return NULL;

	res = p3dc_new_type(P3DC_POLY);
	if (res == NULL)
		return NULL;

	initialize_polygon(res, color, nVerts, vrtx);
	return res;
}

/*
 * Free a polygon and all of its vertices.
 */
void
p3dc_free_polygon(p3dc_POLY *poly) {
	p3dc_free_list(&(poly->pn));
	p3dc_free_list(poly->clip);
	p3dc_free_type(poly->clip);
	p3dc_free_type(poly);
}

/*
 * Create a new p3dc_POLY structure using a color and an array
 * of points in 3D space. The points must be co-planar, and the
 * polygon must be convex! Remember these are PRIMITIVES...
 */
p3dc_FACE *
xxp3dc_new_face(p3dc_CLR *color, int nVerts, p3dc_PNT3 vrtx[], p3dc_PNT3 nrmls[]) {
	p3dc_FACE *res;
	p3dc_NODE *n;
	int i;

	if (nVerts < 3)
		return NULL;

	res = (p3dc_FACE *)p3dc_new_type(P3DC_FACE);
	if (res == NULL)
		return NULL;

	initialize_polygon(&(res->poly), color, nVerts, vrtx);
	p3dc_init_list(&(res->vnorm), P3DC_PNT3, P3DC_LIST_LINKED);
	for (i = 0; i < nVerts; i++) {
		p3dc_PNT3 *t = (p3dc_PNT3 *)p3dc_new_type(P3DC_PNT3);
		n = p3dc_new_node(t, NULL, P3DC_NODE_FREEPAYLOAD);
		*t = nrmls[i];
		/* NOTE: Same order as vertices were created */
		p3dc_add_node(&(res->vnorm), n, P3DC_LIST_TAIL);
	}
	p3dc_init_list(&(res->txtr), P3DC_TXTR, P3DC_LIST_LINKED);
	res->ccode = 0;
	res->cid = 0;
	return res;
}

void
xxp3dc_free_face(p3dc_FACE *f) {
//	if (f->clip != NULL)
//		p3dc_free_face(f->clip);
//	f->clip = NULL;
	p3dc_free_list(&(f->txtr));
	p3dc_free_list(&(f->vnorm));
	p3dc_free_list(f->lights);
	p3dc_free_list(&(f->poly.pn));
	p3dc_free_list(f->poly.clip);
	p3dc_free_type(f);
}

