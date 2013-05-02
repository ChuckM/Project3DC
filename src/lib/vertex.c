/*
 * vertex.c - Vertex management functions for P3DC
 * $Id: vertex.c,v 1.1 1999-08-17 18:34:04-07 cmcmanis Exp $
 *
 * This module implements the vertex functions for p3dc
 *
 * Change Log:
 *	$Log: vertex.c,v $
 *	Revision 1.1  1999/08/18 01:34:04  cmcmanis
 *	Initial revision
 *
 *	Revision 1.1  1999/08/18 01:06:24  cmcmanis
 *	Initial revision
 *
 *	Revision 1.11  1999/08/16 09:51:29  cmcmanis
 *	Woo-hoo. Textures work again.
 *
 *	Revision 1.10  1999/07/26 03:10:23  cmcmanis
 *	Renaming is complete, ModelTest runs again, the view was added to support Models sharing the same draw functions as other programs.
 *
 *	Revision 1.9  1999/07/25 09:11:57  cmcmanis
 *	The great renaming from g3d/G3D -> p3dc/P3DC
 *
 *	Revision 1.8  1998/11/16 03:41:07  cmcmanis
 *	Updated clipping
 *
 *	Revision 1.8  1998/11/08 05:22:41  cmcmanis
 *	Outdated (renamed to 'H:\P3DC-RCS\src\lib\vertex.c,v').
 *
 *	Revision 1.7  1998/10/18 09:29:39  cmcmanis
 *	Models now position and move correctly. I've re-ordered my matrices
 *	to be SPHIGs compatible so that I can use SPHIGS to test my results.
 *	Created p3dc_camera_xfrm and p3dc_orientation_xfrm as compliments of
 *	each other. Cool stuff there.
 *
 *	Revision 1.6  1998/10/10 07:00:47  cmcmanis
 *	Model parsing is written (not fully debugged) and the texture structures
 *	are closer to their final form. By the next check in we should have a
 *	few simple models up and running with textured models to follow.
 *
 *	Revision 1.5  1998/09/23 08:49:02  cmcmanis
 *	Clipping works! Polygon and line clipping is pretty robust at this point
 *	still looking for corner cases but I believe all of the code is correct at
 *	this point.
 *
 *	Revision 1.4  1998/09/20 08:11:02  cmcmanis
 *	Major rewrite, added list stuff, and primitives. Almost completely
 *	back to where version 4 was at this point. This version is ALPHA0_2
 *
 *	Revision 1.3  1998/09/04 05:47:57  cmcmanis
 *	Lighting works for lines and points.
 *	New memory allocator is in place but not
 *	heavily used yet.
 *
 *	Revision 1.2  1998/09/03 19:52:16  cmcmanis
 *	Cube Test runs, camera and transformation code checks out, 
 *  lighting still does not work correctly.
 *
 *	Revision 1.1  1998/09/02 08:48:30  cmcmanis
 *	Initial revision
 *
 *	Revision 1.2  1998/08/27 07:56:30  cmcmanis
 *	New clip architecture, added the NLST allocator/deallocator
 *
 *	Revision 1.1  1998/08/24 16:36:07  cmcmanis
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

/*
 * Some house keeping totals.
 */
int __p3dc_total_vrtx;
int __p3dc_xfrmd_vrtx;

/*
 * Return a new vertex structure. Only the x, y, z, w, and xid
 * values are guaranteed, everything else is junk. This tries
 * to avoid malloc() if possible.
 */
p3dc_VRTX *
p3dc_new_vrtx(p3dc_FLOAT x, p3dc_FLOAT y, p3dc_FLOAT z) {
	p3dc_VRTX *r;

	r = (p3dc_VRTX *)p3dc_new_type(P3DC_VRTX);
	memset((char *)r, 0, sizeof(p3dc_VRTX));
	r->v.x = x; r->v.y = y; r->v.z = z;
	r->xid = 0;
	return r;
}

/*
 * Free up a vertex structure for re-use. Puts the vertex on the
 * 'free' pool and clears the xid.
 */
void
p3dc_free_vrtx(p3dc_VRTX **v) {
	p3dc_free_type(*v);
	*v = NULL;
}


/* Print a vertex to std out */
void
__p3dc_print_vrtx(p3dc_VRTX *a) {
    printf("[ %10.5f  %10.5f  %10.5f]\n",
		a->v.x, a->v.y, a->v.z);
    if (((a->tv.w) != 1.0) && ! EQZ(a->tv.w))
		printf("N = [ %10.5f  %10.5f  %10.5f  %10.5f ]\n",
					a->tv.x/a->tv.w, a->tv.y/a->tv.w, a->tv.z/a->tv.w, a->tv.w); 
}

/* 
 * Transform a vertex by the passed transform, leave result in transformed
 * vertex values (tx, ty, tz);
 *
 * Note: We compute 'w' although that is only really used when we're converting
 * to screen co-ordinates. We should probably be smart about that.
 */
void
p3dc_xfrm_vrtx(p3dc_VRTX *r, p3dc_XFRM *t, p3dc_VRTX *src) {
	p3dc_FLOAT oow;
	__p3dc_total_vrtx++;
	
	if (src->xid != t->id) {
		__p3dc_xfrmd_vrtx++;
		src->tv.x = src->v.x * t->data[0][0] + src->v.y * t->data[0][1] + 
					src->v.z * t->data[0][2] + t->data[0][3];
		if (EQZ(src->tv.x))
			src->tv.x = 0;

		src->tv.y = src->v.x * t->data[1][0] + src->v.y * t->data[1][1] + 
					src->v.z * t->data[1][2] + t->data[1][3];
		if (EQZ(src->tv.y))
			src->tv.y = 0;

		src->tv.z = src->v.x * t->data[2][0] + src->v.y * t->data[2][1] + 
					src->v.z * t->data[2][2] + t->data[2][3];
		if (EQZ(src->tv.z)) 
			src->tv.z = 0;

		if (t->data[3][3] == 1.0) {
			src->tv.w = 1.0f;
			src->ccode = 0;
		} else {
			src->tv.w = src->v.x * t->data[3][0] + src->v.y * t->data[3][1] + 
						src->v.z * t->data[3][2] + t->data[3][3];
			if (EQZ(src->tv.w)) 
				src->tv.w = 0;

			/*
			 * If we did compute w then we'll probably clip this later so
			 * compute the outcode and store it.
			 */
			src->ccode = 0;
			if (src->tv.x <= -(src->tv.w))
				src->ccode |= (1 << NEG_X_PLANE);
			if (src->tv.x >= src->tv.w)
				src->ccode |= (1 << POS_X_PLANE);
			if (src->tv.y <= - (src->tv.w))
				src->ccode |= (1 << NEG_Y_PLANE);
			if (src->tv.y >= src->tv.w)
				src->ccode |= (1 << POS_Y_PLANE);
			if (src->tv.z <= 0)
				src->ccode |= (1 << NEG_Z_PLANE);
			if (src->tv.z >= src->tv.w)
				src->ccode |= (1 << POS_Z_PLANE);
		}
		src->xid = t->id;
	}

	if (r == NULL) 
		return;

    oow = 1.0f/src->tv.w;
	r->v.x = src->tv.x * oow;
	r->v.y = src->tv.y * oow;
	r->v.z = src->tv.z * oow;
	r->xid = 0;
}

/*
 * Find the cross product of two vertices presumed to be at the origin.
 * Note we go to some lengths to allow the 'result' vertex to be the
 * same pointer as either A or B.
 */
p3dc_VRTX *
p3dc_cross_vrtx(p3dc_VRTX *r, p3dc_VRTX *a, p3dc_VRTX *b) {
	p3dc_FLOAT bx, by, bz;
	p3dc_FLOAT ax, ay, az;

	bx = b->v.x; by = b->v.y; bz = b->v.z;
	ax = a->v.x; ay = a->v.y; az = a->v.z;
	if (r == NULL) {
	    r = p3dc_new_vrtx(ay*bz - az*by, az*bx - ax*bz, ax*by - ay*bx);
	} else {
		r->v.x = ay*bz - az*by;
		r->v.y = az*bx - ax*bz;
		r->v.z = ax*by - ay*bx;
		r->xid = 0;
	}
	return (r);
}

/*
 * Subtract two vertices.
 */
p3dc_VRTX *
p3dc_sub_vrtx(p3dc_VRTX *r, p3dc_VRTX *a, p3dc_VRTX *b) {
	if (r == NULL) {
	    r = p3dc_new_vrtx(b->v.x - a->v.x, b->v.y - a->v.y, b->v.z - a->v.z);
	} else {
		r->v.x = b->v.x - a->v.x;
		r->v.y = b->v.y - a->v.y;
		r->v.z = b->v.z - a->v.z;
		r->xid = 0;
	}
	return (r);
}

/*
 * Compute cross product (perpindicular vector) at the stated origin.
 */
p3dc_VRTX *
p3dc_cross3_vrtx(p3dc_VRTX *r, p3dc_VRTX *o, p3dc_VRTX *a, p3dc_VRTX *b) {
	p3dc_VRTX v1, v2;
	if (r == NULL)
		r = p3dc_new_vrtx(0, 0, 0);

	p3dc_sub_vrtx(&v1, a, o);
	p3dc_sub_vrtx(&v2, b, o);
	return (p3dc_cross_vrtx(r, &v1, &v2));
}

/*
 * compute the dot product of a vertex
 */
p3dc_FLOAT
p3dc_dot_vrtx(p3dc_VRTX *a, p3dc_VRTX *b) {
	return (a->v.x*b->v.x + a->v.y*b->v.y + a->v.z*b->v.z);
}

/*
 * compute the dot product of two vectors located at origin 'o'
 */
p3dc_FLOAT
p3dc_dot3_vrtx(p3dc_VRTX *o, p3dc_VRTX *a, p3dc_VRTX *b) {
	return ((a->v.x - o->v.x) * (b->v.x - o->v.x) +
			(a->v.y - o->v.y) * (b->v.y - o->v.y) +
			(a->v.z - o->v.z) * (b->v.z - o->v.z));
}

/*
 * Convert the vertex to be a unit vector
 */
p3dc_VRTX *
p3dc_unit_vrtx(p3dc_VRTX *r, p3dc_VRTX *p) {
	p3dc_FLOAT d = (p3dc_FLOAT) sqrt(p3dc_dot_vrtx(p, p));
	p3dc_FLOAT	ood;

	if (EQZ(d))
		return NULL; 
	ood = 1/d;
	if (r == NULL) {
		r = p3dc_new_vrtx(p->v.x*ood, p->v.y*ood, p->v.z*ood);
	} else {
		r->v.x = p->v.x*ood;
		r->v.y = p->v.y*ood;
		r->v.z = p->v.z*ood;
	}
	return r;
}

/*
 * Compute the unit vector normal to the plane formed
 * by points a, b, and c. We assume that a, b and c
 * are in counter-clockwise order.
 */
p3dc_VRTX *
p3dc_normal_vrtx(p3dc_VRTX *r, p3dc_VRTX *a, p3dc_VRTX *b, p3dc_VRTX *c) {
	p3dc_VRTX *t = p3dc_cross3_vrtx(r, a, b, c);
	return (p3dc_unit_vrtx(t, t));
}

/*
 * Scales the vertex by the passed amount.
 */
p3dc_VRTX *
p3dc_scale_vrtx(p3dc_VRTX *r, p3dc_VRTX *v, p3dc_FLOAT sc) {
	if (r == NULL)
	    r = p3dc_new_vrtx(v->v.x * sc, v->v.y * sc, v->v.z * sc);
	else {
		r->v.x = v->v.x * sc;
		r->v.y = v->v.y * sc;
		r->v.z = v->v.z * sc;
	}
	return (r);
}

/*
 * Add two vertices.
 */
p3dc_VRTX *
p3dc_add_vrtx(p3dc_VRTX *r, p3dc_VRTX *a, p3dc_VRTX *b) {
	if (r == NULL) {
	    r = p3dc_new_vrtx(b->v.x + a->v.x, b->v.y + a->v.y, b->v.z + a->v.z);
	} else {
		r->v.x = b->v.x + a->v.x;
		r->v.y = b->v.y + a->v.y;
		r->v.z = b->v.z + a->v.z;
		r->xid = 0;
	}
	return (r);
}


