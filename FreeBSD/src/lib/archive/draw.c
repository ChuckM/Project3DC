/*
 * draw.c - This file contains the rendering functions. The output
 *			of them will be sent to the rendering hardware.
 *
 * $Id: draw.c,v 1.2 1999/09/02 03:22:37 cmcmanis Exp cmcmanis $
 *
 * As with other parts of the library this module deals with the four
 * basic P3DC types, points, lines, polygons, and faces. 
 *
 * I decided on 8/28/98 that point, lines, and polygons were not going
 * to support Gourad shading. These are simple elements and thus there
 * isn't really anywhere to keep the linkage information. Faces however
 * are full blown textured and shaded polygons.
 *
 * Change Log :
 *	$Log: draw.c,v $
 *	Revision 1.2  1999/09/02 03:22:37  cmcmanis
 *	The great LIST rewrite, lists are much cleaner now
 *
 *	Revision 1.1  1999/08/18 01:33:41  cmcmanis
 *	Initial revision
 *
 *	Revision 1.1  1999/08/18 01:06:00  cmcmanis
 *	Initial revision
 *
 *	Revision 1.12  1999/08/16 09:51:22  cmcmanis
 *	Woo-hoo. Textures work again.
 *
 *	Revision 1.11  1999/08/14 23:18:36  cmcmanis
 *	This version now includes a new model structure that allows
 *	for dynamic changing of colors and textures in the model
 *	after it has been loaded.
 *
 *	Revision 1.10  1999/07/26 03:10:25  cmcmanis
 *	Renaming is complete, ModelTest runs again, the view was added to 
 *	support Models sharing the same draw functions as other programs.
 *
 *	Revision 1.9  1999/07/25 09:11:51  cmcmanis
 *	The great renaming from g3d/G3D -> p3dc/P3DC
 *
 *	Revision 1.8  1998/11/16 03:40:51  cmcmanis
 *	Updated clipping
 *
 *	Revision 1.8  1998/11/08 05:18:55  cmcmanis
 *	Outdated (renamed to 'H:\P3DC-RCS\src\lib\draw.c,v').
 *
 *	Revision 1.7  1998/10/18 09:29:41  cmcmanis
 *	Models now position and move correctly. I've re-ordered my matrices
 *	to be SPHIGs compatible so that I can use SPHIGS to test my results.
 *	Created p3dc_camera_xfrm and p3dc_orientation_xfrm as compliments of
 *	each other. Cool stuff there.
 *
 *	Revision 1.6  1998/10/10 07:00:31  cmcmanis
 *	Model parsing is written (not fully debugged) and the texture structures
 *	are closer to their final form. By the next check in we should have a
 *	few simple models up and running with textured models to follow.
 *
 *	Revision 1.5  1998/09/23 08:48:45  cmcmanis
 *	Clipping works! Polygon and line clipping is pretty robust at this point
 *	still looking for corner cases but I believe all of the code is correct at
 *	this point.
 *
 *	Revision 1.4  1998/09/20 08:10:47  cmcmanis
 *	Major rewrite, added list stuff, and primitives. Almost completely
 *	back to where version 4 was at this point. This version is ALPHA0_2
 *
 *	Revision 1.3  1998/09/18 08:34:35  cmcmanis
 *	This update has new clipping code for polygons.
 *	In particular the nodes are not needlessly copied.
 *
 *	Adds the new list capability in util.c/memory.c that
 *	uses a p3dc_NLST (type P3DC_LIST) as a simple
 *	linked list.
 *
 *	Revision 1.2  1998/09/03 19:52:03  cmcmanis
 *	Cube Test runs, camera and transformation code checks out, 
 *  lighting still does not work correctly.
 *
 *	Revision 1.1  1998/09/02 08:48:18  cmcmanis
 *	Initial revision
 *
 *	Revision 1.1  1998/08/30 09:46:47  cmcmanis
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


int wantStats = 0;
int __p3dc_total_parts;
int __p3dc_drawn_parts;

/*
 * This utility routine is in place so that I can verify that
 * the normals on my vertices are correct.
 */
int __p3dc_normal_view = 0;
int __p3dc_forced_wire_view = 0;
p3dc_FLOAT __p3dc_normal_size = 1.5;
static p3dc_CLR nrml_clr = {0xffff00ff}; /* yellow */

static void
draw_normal(p3dc_VIEW *vp, p3dc_XFRM *view, 
			p3dc_FLOAT x, p3dc_FLOAT y, p3dc_FLOAT z, 
			p3dc_FLOAT nx, p3dc_FLOAT ny, p3dc_FLOAT nz) {
	p3dc_VRTX b, c;
	p3dc_LINE nl;
	
	memset((char *)&b, 0, sizeof(b));
	memset((char *)&c, 0, sizeof(c));
	memset((char *)&nl, 0, sizeof(nl));

	b.v.x = nx; b.v.y = ny; b.v.z = nz;
	p3dc_unit_vrtx(&b, &b);
	c.v.x = (b.v.x * __p3dc_normal_size) + x;
	c.v.y = (b.v.y * __p3dc_normal_size) + y;
	c.v.z = (b.v.z * __p3dc_normal_size) + z;
	b.v.x += x;
	b.v.y += y;
	b.v.z += z;
	nl.p1 = &b;
	nl.p2 = &c;
	nl.color = &nrml_clr;
	nl.light.x = nl.light.y = nl.light.z = nl.light.w = 1.0f;

	p3dc_draw_line(vp, view, P3DC_DRAW_NOSHADE, &nl);
}


/*
 * Draw a point on the screen using the grDrawPoint primitive.
 * This is computationally pretty expensive, we should access the
 * frame buffer directly, but since I don't draw that many points
 * I've not taken the time to optimize this.
 */
void
p3dc_draw_point(p3dc_VIEW *vp, p3dc_XFRM *view, int flags, p3dc_POINT *p) {
	__p3dc_total_parts++;
	if ((flags & P3DC_DRAW_NOSHADE) == 0) {
		p3dc_light_point(&(vp->origin), p);
	} 

	p3dc_xfrm_vrtx(NULL, view, p->p0);
	if (((flags & P3DC_DRAW_NOCLIP) == 0) && (p3dc_clip_point(p) == CLIP_NONE))
		return;
	__p3dc_drawn_parts++;
	__hw_draw_point(vp, p);
}

/*
 * Draw a line in the frame buffer. The vertices are assumed to be in
 * world co-ordinates and are transformed into view co-ordinates using
 * the P matrix in the camera.
 */
void
p3dc_draw_line(p3dc_VIEW *vp, p3dc_XFRM *view, int flags, p3dc_LINE *a) {
	p3dc_LINE cLine; 
	p3dc_VRTX v1, v2;
	float d1, d2; 
	int code = CLIP_A_B;

	__p3dc_total_parts++;

	/* Start of the pixel pipeline: Light the vertices.*/
	if ((flags & P3DC_DRAW_NOSHADE) == 0) {
		p3dc_light_line(&(vp->origin), a);
	} 

	/* Stage 2: Transform from world space into view space */
	p3dc_xfrm_vrtx(NULL, view, a->p1);
	p3dc_xfrm_vrtx(NULL, view, a->p2);

	/* Stage 3: Clip the line. */
	if ((flags & P3DC_DRAW_NOCLIP) == 0)
		code = p3dc_clip_line(a, &d1, &d2);

	/* Stage 4a: Either pass it straight to the hardware or drop it. */
	if (code == CLIP_NONE)
		return;
	else if (code == CLIP_A_B) {
		__hw_draw_line(vp, a);
		return;
	}

	/*
	 * Stage 4b: The line was clipped so create a new line struct with
	 * the clipped values in it and pass _that_ to the hardware drawing
	 * engine. This way the hardware engine never sees anything that
	 * isn't "in the box" of the viewing rectangle.
	 */
	v1 = *(a->p1);
	v2 = *(a->p2);
	cLine.p1 = &v1;
	cLine.p2 = &v2;
	cLine.light = a->light;
	cLine.color = a->color;
	cLine.p1->tv = a->c1;
	cLine.p2->tv = a->c2;
	__hw_draw_line(vp, &cLine);
	__p3dc_drawn_parts++;
	return;
}

/* 
 * Ok now for something rather much more difficult, lets draw a
 * polygon.
  */
void
p3dc_draw_polygon(p3dc_VIEW *vp, p3dc_XFRM *view, int flags, p3dc_POLY *poly) {
	p3dc_NODE	*t;

	__p3dc_total_parts++;
	if ((flags & P3DC_DRAW_NOSHADE) == 0) {
		p3dc_light_polygon(poly);
	} 

	/* Transform vertices */ 
	if (poly->xid != view->id) {
		for (t = poly->pn.head; t != NULL; t = t->nxt) {
			p3dc_xfrm_vrtx(NULL, view, (p3dc_VRTX *)(t->data.p));
		}
		poly->cid = 0;
		poly->xid = view->id;
	}

	__p3dc_drawn_parts++;
	/* If it clips off the screen return */
	if ((flags & P3DC_DRAW_NOCLIP) == 0) {
		if (poly->cid != poly->xid) {
			p3dc_clip_free_cliplist(&(poly->clip));
			poly->ccode = p3dc_clip_lists(&(poly->pn), &(poly->clip), 1);
			poly->cid = poly->xid; 
		}
		if (poly->ccode == CLIP_NONE)
			return;
		else if (poly->ccode != CLIP_A_B) {
			__hw_draw_polygon(vp, poly, &(poly->clip));
			return;
		}
	}
	__hw_draw_polygon(vp, poly, &(poly->pn));
}

/* 
 * Drawing "faces" is where the stuff that looks like real video games is done.
 *
 * Unlike polygons, face structures can have normals associated with their vertices,
 * and light maps and textures. Each of these things needs to be dealt with before 
 * rendering can begin.
 */
void
p3dc_draw_face(p3dc_VIEW *vp, p3dc_XFRM *view, p3dc_XFRM *world, int flags, p3dc_FACE *face) {
	p3dc_NODE	*t, *t2;
	int num_lists;
	p3dc_POLY	*poly = &(face->poly);
	void *texture = NULL;

	/* XXX 
	 * This is a weakness, the code transforms all of the face vertex
	 * normals here, instead of transforming the lights into model
	 * space and lighting the polygons there. It does use the vertex
	 * accellerators for transforms to note if it doesn't have to do
	 * it more than once.
	 *
	 * TODO: Figure out how to keep the models as distinct pre-lit objects
	 *	that require no additional transformations.
	 *
	 * Note: One way to do this will be to pre-light the models before
	 * rendering them. Since this routine accellerates when the light id of
	 * the face matches the global light id, this would make sense.
	 *
	 */ 
	for (t = face->vnorm.head, t2 = poly->pn.head; t != NULL; t = t->nxt, t2 = t2->nxt) {
		p3dc_VRTX *vv = (t->data.p);
		p3dc_VRTX *xx = (t2->data.p);
		if (vv->xid == world->id) {
			if (__p3dc_normal_view) {
				draw_normal(vp, view, xx->v.x, xx->v.y, xx->v.z,
						vv->v.x, vv->v.y, vv->v.z);
			}
			continue;
		}

		/* We do our own transform since normals have no w value */
		vv->tv.x = vv->v.x * world->data[0][0] +
				  		vv->v.y * world->data[0][1] +
						vv->v.z * world->data[0][2];
		vv->tv.y = vv->v.x * world->data[1][0] +
						vv->v.y * world->data[1][1] +
						vv->v.z * world->data[1][2];
		vv->tv.z = vv->v.x * world->data[2][0] +
						vv->v.y * world->data[2][1] +
						vv->v.z * world->data[2][2];
		vv->xid = world->id;
		if (__p3dc_normal_view) {
			draw_normal(vp, view, xx->v.x, xx->v.y, xx->v.z,
					vv->v.x, vv->v.y, vv->v.z);
		}
		face->lid = 0;
	}

	num_lists = 1; /* Always clip the vertices */
	if ((flags & P3DC_DRAW_NOSHADE) == 0) {
		p3dc_light_face(face);
		num_lists = 2; /* Now also clip the lights */
	} 

	/* Transform vertices */ 
	if (poly->xid != view->id) {
		for (t = poly->pn.head; t != NULL; t = t->nxt) {
			/* this automatically takes advantage of the xfrm accellerator */
			p3dc_xfrm_vrtx(NULL, view, (p3dc_VRTX *)(t->data.p));
		}
		poly->cid = 0;
		face->xid = poly->xid = view->id;
		face->cid = 0;
		face->ancil[P3DC_FACE_VERTEX] = face->poly.pn;
	}

	if (face->txtr.head) {
		p3dc_TXTR *tx = (p3dc_TXTR *)(face->txtr.head->data.p);
		face->ancil[P3DC_FACE_BASE] = tx->vrtx;
		num_lists = 3; /* Now also clip it's texture */
		texture = tx->tex->data;
	}

	/* If it clips off the screen return */
	if ((flags & P3DC_DRAW_NOCLIP) == 0) {
		if (face->cid != face->xid) {
			/* XXX TODO: Currently is only clipping lights and vertices, base txtr */
			face->ccode = p3dc_clip_lists(face->ancil, face->clip, num_lists);
			face->cid = face->xid; 
		}
		if (face->ccode == CLIP_NONE)
			return;
		else if (face->ccode != CLIP_A_B) {
			__hw_draw_face(vp, face, face->clip, texture);
			return;
		}
	}
	__hw_draw_face(vp, face, face->ancil, texture);
}

static int mboxx[12] = {0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3 };
static int mboxy[12] = {1, 2, 3, 0, 5, 6, 7, 4, 6, 5, 4, 7 };

void
p3dc_draw_model_bbox(p3dc_VIEW *vp, p3dc_CLR *cc, int flags, p3dc_MODEL *m) {
	int i;
	p3dc_NODE *t;
	p3dc_LINE *tL;

	if (m->box.head == NULL) {
		for (i = 0; i < 12; i++) {
			if (cc == NULL) cc = &(m->color);
			tL = p3dc_new_line(cc, &(m->corners[mboxx[i]]), 
									&(m->corners[mboxy[i]]));
			p3dc_add_node(&(m->box), p3dc_new_node(tL, NULL, 0), P3DC_LIST_TAIL);
		}
	} else if (cc != NULL) {
		for (t = FIRST_NODE(&(m->box)); t != NULL; t = NEXT_NODE(&(m->box))) {
			tL = (p3dc_LINE *)(t->data.p);
			*(tL->color) = *(cc);
		}
	}
	p3dc_draw_list(vp, &(m->V), flags, &(m->box));
}

/*
 * This is the top level function for drawing a model, I thought about
 * leaving it in model.c but its a drawing function more than a model
 * manipulation function.
 */
void
p3dc_draw_model(p3dc_VIEW *vp, p3dc_XFRM *view, int flags, p3dc_MODEL *m) {
	p3dc_NODE *n;

	/*
	 * if the Transform Valid flag isn't set then we need
	 * to recompute the model to camera transform.
	 */
	if ((m->flags == 0) || (m->xid != view->id)) {
		p3dc_mult_xfrm(&(m->V), view, &(m->O));
		m->xid = view->id;
		m->flags++;
	}

	for (n = m->components.head; n != NULL; n = n->nxt) {
		switch (n->data.t) {
			case P3DC_POINT:
				p3dc_draw_point(vp, &(m->V), flags, (p3dc_POINT *)(n->data.p));
				break;
			case P3DC_LINE:
				p3dc_draw_line(vp, &(m->V), flags, (p3dc_LINE *)(n->data.p));
				break;
			case P3DC_FACE:
				p3dc_draw_face(vp, &(m->V), &(m->O), flags, (p3dc_FACE *)(n->data.p));
				break;
			default:
				p3dc_log("Bogus component in the model!\n");
				return;
		}
	}
	if (flags & P3DC_DRAW_BBOX)
		p3dc_draw_model_bbox(vp, &COLOR_red, flags, m);
}

#if 0

/*
 * Ok, the big kahuna, the triangle drawer. This function draws a flat
 * shaded triangle using the color of the triangle as its base color.
 * The color is modulated by the light source. (light color is white)
 */
void
p3dc_draw_triangle(p3dc_CMRA *e, p3dc_CLR *color, float shade,
			  p3dc_VRTX *pa, p3dc_VRTX *pb, p3dc_VRTX *pc, 
			  p3dc_VRTX *n, int filled, int flags) {
	p3dc_VRTX p[12];
	int nTri, i;
	int nWasValid = 0;
	int clip_it = (flags & P3DC_CLIPPED_PART) != 0;
	int shade_it = (flags & P3DC_SHADED_PART) != 0;

	__p3dc_total_parts++;
	p3dc_xfrm_vrtx(NULL, &(e->P), pa);
	p3dc_xfrm_vrtx(NULL, &(e->P), pb);
	p3dc_xfrm_vrtx(NULL, &(e->P), pc);

	/* XXX need to eliminate this copy XXX */
	p[0] = *pa; p[1] = *pb; p[2] = *pc;
	nWasValid = (pa->flags & pb->flags & pc->flags & p3dc_N_valid) != 0;
	if (clip_it == 0) {
		nTri = 1;
	} else {
		nTri = p3dc_clip_triangle(p, pa, pb, pc);
		if (nTri == 0) {
			return; /* nothing to do, triangle is out of view */
		}
	}

	/* if you want to see the normals, we draw them in first */
	if (__p3dc_normal_view) {
		p3dc_FLOAT lx, ly, lz;
		lx = (pa->x+pb->x+pc->x) / (p3dc_FLOAT) 3.0;
		ly = (pa->y+pb->y+pc->y) / (p3dc_FLOAT) 3.0;
		lz = (pa->z+pb->z+pc->z) / (p3dc_FLOAT) 3.0;
		draw_normal(e, lx, ly, lz, n->x, n->y, n->z, &COLOR_blue);
		if (pa->flags & p3dc_N_valid) 
			draw_normal(e, pa->x, pa->y, pa->z, pa->nx, pa->ny, pa->nz, &COLOR_green);
		if (pb->flags & p3dc_N_valid) 
			draw_normal(e, pb->x, pb->y, pb->z, pb->nx, pb->ny, pb->nz, &COLOR_green);
		if (pc->flags & p3dc_N_valid) 
			draw_normal(e, pc->x, pc->y, pc->z, pc->nx, pc->ny, pc->nz, &COLOR_green);
	}

	/* Clipping may have returned more triangles to us */
	pa = &p[0];
	for (i = 0; i < nTri; i++) {
		__p3dc_drawn_parts++;
		pb = &p[i+1];
		pc = &p[i+2];
		if (filled & 1)
			__hw_draw_triangle(e, color, shade, pa, pb, pc, shade_it);
		if (filled & 2) {
			p3dc_CLR *tc = color;
			if (filled & 1) {
				tc = &COLOR_red;
				if (__p3dc_forced_wire_view)
					p3dc_display_ctrl(P3DC_CTRL_DEPTHBUFFER, 0);
			}
				
			__hw_draw_line(e, tc, shade, pa, pb, shade_it);
			__hw_draw_line(e, tc, shade, pb, pc, shade_it);
			__hw_draw_line(e, tc, shade, pc, pa, shade_it); 
			if ((filled & 1) && __p3dc_forced_wire_view) {
				p3dc_display_ctrl(P3DC_CTRL_DEPTHBUFFER, 1);
			}
		}
	}
}


#endif

static struct line {
	p3dc_CLR		*c;	/* Line color */
	p3dc_PNT3	s;	/* Start point */
	p3dc_PNT3	e;	/* End point */
} axe_data[] = {
	{ &COLOR_green, {0, 0, 0}, {15, 0, 0}},
	{ &COLOR_green, {14, 1, 0}, {15, 0, 0}},
	{ &COLOR_green, {14, -1, 0}, {15, 0, 0}},
	{ &COLOR_green, {16, 2, 0}, {18, -2, 0}},
	{ &COLOR_green, {16, -2, 0}, {18, 2, 0}},
	{ &COLOR_red, {0, 0, 0}, {0, 15, 0}},
	{ &COLOR_red, {-1, 14, 0}, {0, 15, 0}},
	{ &COLOR_red, {1, 14, 0}, {0, 15, 0}},
	{ &COLOR_red, {1, 18, 0}, {-1, 16, 0}},
	{ &COLOR_red, {-1, 18, 0}, {0, 17, 0}},
	{ &COLOR_blue, {0, 0, 0}, {0, 0, 15}},
	{ &COLOR_blue, {-1, 0, 14}, {0, 0, 15}},
	{ &COLOR_blue, {1, 0, 14}, {0, 0, 15}},
	{ &COLOR_blue, {-1, -1, 16}, {1, 1, 16}},
	{ &COLOR_blue, {1, -1, 16}, {-1, -1, 16}},
	{ &COLOR_blue, {1, 1, 16}, {-1, 1, 16}},
	{ NULL, {0, 0, 0}, {0, 0, 0}}
};

static p3dc_LIST axis_lines = { NULL, NULL, P3DC_LINE };
static p3dc_FLOAT axis_scale = 0;

/*
 * Draw the major cartesian axes into the world.
 */
void
p3dc_draw_axes(p3dc_VIEW *vp, p3dc_XFRM *view, int flags, p3dc_FLOAT scale) {
	int i = 0;
	p3dc_NODE *n;

	if ((axis_lines.head == NULL) || (axis_scale != scale)) {
		axis_lines.n_type = P3DC_LINE;
		axis_lines.l_type = P3DC_LIST_LINKED;
		if (axis_lines.head != NULL)
			p3dc_free_list(&axis_lines);
		axis_scale = scale;
		while(axe_data[i].c) {
			p3dc_LINE *ll;
			p3dc_PNT3 strt, end;
			strt = axe_data[i].s;
			end = axe_data[i].e;
			strt.x *= scale;
			strt.y *= scale;
			strt.z *= scale;
			end.x *= scale;
			end.y *= scale;
			end.z *= scale;

			ll = p3dc_new_line(axe_data[i].c, &strt, &end);
			n = p3dc_new_node(ll, NULL, P3DC_NODE_FREEPAYLOAD);
			p3dc_add_node(&axis_lines, n, P3DC_LIST_TAIL);
			i++;
		}
	}
	p3dc_draw_list(vp, view, flags, &axis_lines);
}

void
p3dc_draw_primitive(p3dc_VIEW *vp, p3dc_XFRM *view, int flags, p3dc_NODE *prim) {
	switch (prim->data.t) {
	case P3DC_LINE:
		p3dc_draw_line(vp, view, flags, (p3dc_LINE *)(prim->data.p));
		break;
	case P3DC_POINT:
		p3dc_draw_point(vp, view, flags, (p3dc_POINT *)(prim->data.p));
		break;
	case P3DC_POLY:
		p3dc_draw_polygon(vp, view, flags, (p3dc_POLY *)(prim->data.p));
		break;
	default:
		p3dc_log("Unknown primitive.\n");
		break;
	}
	return;
}

void
p3dc_draw_list(p3dc_VIEW *vp, p3dc_XFRM *view, int flags, p3dc_LIST *prims) {
	p3dc_NODE *t;
	for (t = prims->head; t != NULL; t = t->nxt) {
		p3dc_draw_primitive(vp, view, flags, t);
	}
}
