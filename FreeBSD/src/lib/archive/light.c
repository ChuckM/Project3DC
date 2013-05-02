/*
 * light.c -- The lighting module for P3DC
 * $Id: light.c,v 1.2 1999/09/02 03:22:38 cmcmanis Exp cmcmanis $
 *
 * This module "lights" an object (point, line, poly,
 * or face) based on the global lighting state. It is
 * initially quite simple.
 *
 * Change History:
 *	$Log: light.c,v $
 *	Revision 1.2  1999/09/02 03:22:38  cmcmanis
 *	The great LIST rewrite, lists are much cleaner now
 *
 *	Revision 1.1  1999/08/18 01:33:43  cmcmanis
 *	Initial revision
 *
 *	Revision 1.1  1999/08/18 01:06:14  cmcmanis
 *	Initial revision
 *
 *	Revision 1.12  1999/08/16 09:51:24  cmcmanis
 *	Woo-hoo. Textures work again.
 *
 *	Revision 1.11  1999/08/14 23:18:37  cmcmanis
 *	This version now includes a new model structure that allows
 *	for dynamic changing of colors and textures in the model
 *	after it has been loaded.
 *
 *	Revision 1.10  1999/07/26 03:10:26  cmcmanis
 *	Renaming is complete, ModelTest runs again, the view was added to support Models sharing the same draw functions as other programs.
 *
 *	Revision 1.9  1999/07/25 09:11:52  cmcmanis
 *	The great renaming from g3d/G3D -> p3dc/P3DC
 *
 *	Revision 1.8  1998/11/16 03:40:53  cmcmanis
 *	Updated clipping
 *
 *	Revision 1.8  1998/11/08 05:19:36  cmcmanis
 *	Outdated (renamed to 'H:\P3DC-RCS\src\lib\light.c,v').
 *
 *	Revision 1.7  1998/11/05 20:23:39  cmcmanis
 *	Checkin prior to moving into a better source tree structure
 *
 *	Revision 1.6  1998/10/10 07:00:35  cmcmanis
 *	Model parsing is written (not fully debugged) and the texture structures
 *	are closer to their final form. By the next check in we should have a
 *	few simple models up and running with textured models to follow.
 *
 *	Revision 1.5  1998/09/23 08:48:40  cmcmanis
 *	Clipping works! Polygon and line clipping is pretty robust at this point
 *	still looking for corner cases but I believe all of the code is correct at
 *	this point.
 *
 *	Revision 1.4  1998/09/20 08:10:53  cmcmanis
 *	Major rewrite, added list stuff, and primitives. Almost completely
 *	back to where version 4 was at this point. This version is ALPHA0_2
 *
 *	Revision 1.3  1998/09/04 05:47:52  cmcmanis
 *	Lighting works for lines and points.
 *	New memory allocator is in place but not
 *	heavily used yet.
 *
 *	Revision 1.2  1998/09/03 19:52:09  cmcmanis
 *	Cube Test runs, camera and transformation code checks out,
 *  lighting still does not work correctly.
 *
 *	Revision 1.1  1998/09/02 08:48:23  cmcmanis
 *	Initial revision
 *
 *	Revision 1.1  1998/08/30 09:46:49  cmcmanis
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

#include <stdlib.h>
#include <math.h>
#include <varargs.h>
#include "p3dc.h"

/*
 * LIGHTING CONSTANTS:
 *	- The default light source points towards the 0,0,0 from a positive +x, +y, -z 
 *	- The default 'diffuse' light constant is 20% (.2)
 */
static p3dc_LIGHT default_lights[2] = {
	{	/* Light 0 is the Sun and shines white from very far away */
		{ 1.0f, 1.0f, 1.0f }, 
		{ -0.57735027f, -0.57735027f, 0.57735027f },
		{ 0, 0, 0},			
		{ 0, 0, 0},			
		0, P3DC_LIGHT_DIRECTIONAL, 
	},{	/* Light 1 is ambient light during daylight (about 20%) */
		{ 1.0f, 1.0f, 1.0f },	 /* white light */
		{ 0, 0, 0 },	 
		{ 0, 0, 0 },	 
		{ 0, 0, 0 },
		0.15f, P3DC_LIGHT_AMBIENT,  
	}
};

p3dc_LIST lights = { NULL, NULL, P3DC_LIGHT}; 

static int light_id = 2;

#define REFLECTED_LIGHT(n) ((p3dc_FLOAT) ( \
							   (n)->x * __p3dc_light_normal.x +\
							   (n)->y * __p3dc_light_normal.y +\
							   (n)->z * __p3dc_light_normal.z ))

#define PNT3_CLIP(a, m) { \
			if ((a)->x > (m)) (a)->x = (m); \
			if ((a)->y > (m)) (a)->y = (m); \
			if ((a)->z > (m)) (a)->z = (m); }

static void
compute_light(p3dc_LIGHT *lite, p3dc_PNT3 *obj, p3dc_PNT3 *nrm, p3dc_PNT3 *res) {
	float prod;
	p3dc_PNT3 tmp;

	switch (lite->type) {
	case P3DC_LIGHT_DIRECTIONAL:
		prod = -p3dc_dot_pnt3(nrm, &(lite->dir));
		if (prod < 0) prod = 0;
		break;
		/* This is broken, it needs to compute a conic section */
	case P3DC_LIGHT_SPOT:
		p3dc_diff_pnt3(&(lite->origin), obj, &tmp);
		prod = -p3dc_dot_pnt3(nrm, &tmp);
		break;
	case P3DC_LIGHT_AMBIENT:
		prod = lite->scale;
		break;
	default:
		prod = 1.0f;
		break;
	}
	res->x = prod * lite->color.x;
	res->y = prod * lite->color.y;
	res->z = prod * lite->color.z;
}

/*
 * Ok, one step up in the complexity scale, this function
 * computes the net effect of all lights on a single vertex
 * given that vertex and a normal to use as the vertex normal.
 * A note for later:
 *	Perhaps the correct thing to do is add all light sources
 *	and then divide by the total, that would average the light
 *	in the scene. But then again, that would make ambient light
 *	contribute "less" based on the number of lights. Needs more
 *	thought. This will do for now.
 */
static void
light_vertex(p3dc_VRTX *v, p3dc_PNT4 *light, p3dc_PNT3 *nrm) {
	p3dc_NODE *ll;
	p3dc_PNT3 shade;

	light->x = light->y = light->z = 0; /* reset the point's light value */
	for (ll = lights.head; ll != NULL; ll = ll->nxt) {
		compute_light((p3dc_LIGHT *)(ll->data.p), &(v->v), nrm, &shade);
		light->x += shade.x;
		light->y += shade.y;
		light->z += shade.z;
	}
	PNT3_CLIP(light, 1.0);
	return;
}
	
/*
 * When lighting points I simulate a sphere (a small sphere :-)
 * and the normal of interest happens to be the one that radiates
 * from the point to the camera's origin. 
 */
void
p3dc_light_point(p3dc_PNT3 *origin, p3dc_POINT *p) {
	p3dc_PNT3 norm;

	if (p->lid == light_id)
		return; /* already lit */

	p3dc_diff_pnt3(&(p->p0->v), origin, &norm);
	if (p3dc_normalize_pnt3(&norm))
		return; /* won't be used since we'll get clipped */

	light_vertex(p->p0, &(p->light), &norm);
	return; 
}

/*
 * Light a line segment, this works by first computing an
 * effective normal. The normal is a unit vector that is
 * perpindicular to the line, in the same plane as the 
 * vector between the camera and the line segment. 
 */
void
p3dc_light_line(p3dc_PNT3 *origin, p3dc_LINE *ll) {
	p3dc_VRTX *p1, *p2;
	p3dc_PNT3 norm, dir, nx;

	if (ll->lid == light_id)
		return;

	p1 = ll->p1;
	p2 = ll->p2;
	/* Compute vector from line endpoint to the camera*/
	p3dc_diff_pnt3(&(p1->v), origin, &nx);
	if (p3dc_normalize_pnt3(&nx)) {
		/* P1 is co-incident with the camera! ouch. */
		p3dc_diff_pnt3(&(p2->v), origin, &nx);
		if (p3dc_normalize_pnt3(&nx)) {
			return;  
		}
		/* compute line segment vector from p2 to p1 */
		p3dc_diff_pnt3(&(p2->v), &(p1->v), &dir);
	} else {
		/* Compute the vector of the line segment */
		p3dc_diff_pnt3(&(p1->v), &(p2->v), &dir);
	}
	if (p3dc_normalize_pnt3(&dir)) {
		light_vertex(p1, &(ll->light), &nx);
		return;
	}

	/*
	 * At this point I have a vector from an end point, 
	 * pointing toward the camera. And a vector along the
	 * line segment from that same endpoint. These define
	 * a plane. 
	 */

	/* 
	 * Now compute the normal for the plane between the 
	 * segment and camera 
	 */
	p3dc_cross_pnt3(&dir, &nx, &norm);
	p3dc_normalize_pnt3(&norm);

	/* Given the plane normal, finding the cross product of
	 * the plane normal and the segment direction vector will
	 * give us a new vector that is perpindicular to the segment.
	 * This new vector is the "virtual" normal for the cylinder
	 * represented by the line segment.
	 */
	p3dc_cross_pnt3(&norm, &dir, &nx);
	p3dc_normalize_pnt3(&nx);
	light_vertex(p1, &(ll->light),&nx);
}


void
p3dc_light_polygon(p3dc_POLY *poly) {

	if (poly->lid == light_id)
		return;
	light_vertex((p3dc_VRTX *)(poly->pn.head->data.p), &(poly->light), &(poly->norm));
	poly->lid = light_id;
}

/*
 * Basic face lighting is a bit more complicated. This code traverses
 * three lists, a light list (stored light values), a normal list (vertex normals)
 * and the vertex list in the polygon.
 */
void
p3dc_light_face(p3dc_FACE *f) {
	p3dc_NODE *vn, *ln, *nn;
	/* int adding_lights = 0; */

	if (f->lid == light_id) {
		return;
	}

#if 0
	if (f->lights.head == NULL) {
		adding_lights = 1;
	}
	if (! adding_lights) {
		ln = f->lights.head;
	} else {
		p3dc_PNT4	*p4 = p3dc_new_type(P3DC_PNT4);
		ln = p3dc_new_node(p4, NULL, P3DC_NODE_FREEPAYLOAD);
	}
#endif
	if (f->lights.head != NULL)
		p3dc_free_list(&(f->lights));

	nn = f->vnorm.head;
	vn = f->poly.pn.head;
	while (vn != NULL) {
		p3dc_PNT4 *tp4 = &(((p3dc_VRTX *)(nn->data.p))->tv); /* technically not needed */
		p3dc_PNT3 tp3;
		p3dc_PNT4 *li = p3dc_new_type(P3DC_PNT4);
		ln = p3dc_new_node(li, NULL, P3DC_NODE_FREEPAYLOAD);

		tp3.x = tp4->x;
		tp3.y = tp4->y;
		tp3.z = tp4->z;
		light_vertex((p3dc_VRTX *)(vn->data.p), (p3dc_PNT4 *)(ln->data.p), &tp3);
#if 0
		if (adding_lights) {
			p3dc_PNT4 *p4;
			p3dc_add_node(&(f->lights), ln, P3DC_LIST_TAIL);
			if (vn->nxt != NULL) {
				p4 = p3dc_new_type(P3DC_PNT4);
				ln = p3dc_new_node(p4, NULL, P3DC_NODE_FREEPAYLOAD);
			}
		} else {
			ln = ln->nxt;
		}
#endif
		p3dc_add_node(&(f->lights), ln, P3DC_LIST_TAIL);
		nn = nn->nxt;
		vn = vn->nxt;
	}
	f->lid = light_id;
	f->ancil[P3DC_FACE_LIGHTS] = f->lights;
	return; 
}

/*
 * Reset the lights by freeing the current light list.
 * This has the effect of returning the structures to the two
 * pools. 
 */
void
p3dc_light_reset() {

	while (lights.head != NULL) {
		p3dc_remove_node(&lights, P3DC_NODE_FIRST);
	}
	/* indicate to caches that the lights have changed */
	light_id++;
}

/*
 * Add a light to the lighting system. This adds the passed in light
 * structure (it makes a copy) into the light list. It will be factored
 * in on the next frame.
 */
void
p3dc_light_add(p3dc_LIGHT *new_light) {
	p3dc_NODE *r;
	p3dc_LIGHT *l;

	l = (p3dc_LIGHT *)p3dc_new_type(P3DC_LIGHT);
	*l = *new_light; /* copy the lighting data */
	r = p3dc_new_node(l, NULL, P3DC_NODE_FREEPAYLOAD);

	/* Now add it into the lights linked list */
	p3dc_add_node(&lights, r, P3DC_LIST_TAIL);

	/* finally, update that we have a new lighting config */
	light_id++;
}

/*
 * Initialize the lighting system to the "default" which has
 * a single directional light (the Sun) shining down toward
 * -x, -y, +z. (This is from over your right shoulder if you
 * are looking at the world down the Z axis)
 */
void
p3dc_light_init() {
	if (lights.head != NULL)
		p3dc_light_reset();
	p3dc_light_add(&default_lights[0]);
	p3dc_light_add(&default_lights[1]);
}
