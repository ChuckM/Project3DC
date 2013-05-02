/*
 * clip.c - implementation for the clipping functions.
 *
 * This file is use to clip transformed vertices to the view port.
 * All of these functions only look at the TRANSFORMED co-ordinates
 * of the vertex so it is import that you transform them with the camera
 * first before calling any of these clipping functions.
 *
 * $Log: clip.c,v $
 * Revision 1.7  1999-10-31 11:23:56-08  cmcmanis
 * Camera rewrite is nearly complete. This seems like a logical
 * milestone
 *
 * Revision 1.6  1999/09/12 06:31:05  cmcmanis
 * Rewrote clipping api (again?)
 *
 * Revision 1.5  1999/09/09 02:39:24  cmcmanis
 * Potential memory leak fixed.
 *
 * Revision 1.4  1999/09/03 03:43:12  cmcmanis
 * Fixed a bug with multiple lists being active
 *
 * Revision 1.3  1999/09/02 04:33:26  cmcmanis
 * More list fixes, red/black lists work better now on delete
 * there were a couple of bugs in the last few nodes to be
 * removed...
 *
 * Revision 1.2  1999/09/02 03:22:36  cmcmanis
 * The great LIST rewrite, lists are much cleaner now
 *
 * Revision 1.1  1999/08/18 01:33:37  cmcmanis
 * Initial revision
 *
 * Revision 1.1  1999/08/18 01:05:58  cmcmanis
 * Initial revision
 *
 * Revision 1.14  1999/08/16 09:51:22  cmcmanis
 * Woo-hoo. Textures work again.
 *
 * Revision 1.13  1999/08/14 23:18:35  cmcmanis
 * This version now includes a new model structure that allows
 * for dynamic changing of colors and textures in the model
 * after it has been loaded.
 *
 * Revision 1.12  1999/07/26 03:10:24  cmcmanis
 * Renaming is complete, ModelTest runs again, the view was added to support 
 * Models sharing the same draw functions as other programs.
 *
 * Revision 1.11  1999/07/25 09:11:51  cmcmanis
 * The great renaming from g3d/G3D -> p3dc/P3DC
 *
 * Revision 1.10  1998/11/16 03:40:48  cmcmanis
 * Updated clipping
 *
 * Revision 1.10  1998/11/08 05:17:52  cmcmanis
 * Outdated (renamed to 'H:\P3DC-RCS\src\lib\clip.c,v').
 *
 * Revision 1.9  1998/11/06 18:02:04  cmcmanis
 * Clipping for faces now works as expected.
 *
 * Revision 1.8  1998/11/05 20:23:37  cmcmanis
 * Checkin prior to moving into a better source tree structure
 *
 * Revision 1.7  1998/10/10 07:00:29  cmcmanis
 * Model parsing is written (not fully debugged) and the texture structures
 * are closer to their final form. By the next check in we should have a
 * few simple models up and running with textured models to follow.
 *
 * Revision 1.6  1998/09/23 08:48:42  cmcmanis
 * Clipping works! Polygon and line clipping is pretty robust at this point
 * still looking for corner cases but I believe all of the code is correct at
 * this point.
 *
 * Revision 1.5  1998/09/20 08:10:42  cmcmanis
 * Major rewrite, added list stuff, and primitives. Almost completely
 * back to where version 4 was at this point. This version is ALPHA0_2
 *
 * Revision 1.4  1998/09/18 08:34:32  cmcmanis
 * This update has new clipping code for polygons.
 * In particular the nodes are not needlessly copied.
 *
 * Adds the new list capability in util.c/memory.c that
 * uses a p3dc_NLST (type P3DC_LIST) as a simple
 * linked list.
 *
 * Revision 1.3  1998/09/08 04:04:56  cmcmanis
 * Clipping for lines and points is now verified, allocation
 * of vertices in the GAL layer is fixed.
 *
 * Revision 1.2  1998/09/04 05:47:49  cmcmanis
 * Lighting works for lines and points.
 * New memory allocator is in place but not
 * heavily used yet.
 *
 * Revision 1.1  1998/09/02 08:48:16  cmcmanis
 * Initial revision
 *
 * Revision 1.2  1998/08/27 07:56:34  cmcmanis
 * New clip architecture, added the NLST allocator/deallocator
 *
 * Revision 1.1  1998/08/25 16:12:13  cmcmanis
 * Initial revision
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
#include <math.h>
#include <string.h>
#include <assert.h>
#include "p3dc.h"

/* #define CLIPPING_DEBUG */
 
typedef struct __clip_context {
	int	active;	/* True if this context is in use */
	p3dc_LIST	*src;
	p3dc_NODE	*first;
	p3dc_NODE	*current;
	p3dc_NODE	*next;
	int			dstNodes; /* for debugging */
	int			srcNodes;
	p3dc_LIST	*dst;
} CLIP_CONTEXT;

/*
 * This function creates a node copy. When clipping the first time
 * around you end up making a copy of the nodes, subsequent clips
 * reuse the nodes in the generated list.
 */
static p3dc_NODE *
copy_node(p3dc_NODE *orig) {
	p3dc_PNT4	*n4;
	p3dc_PNT3	*n3;
	p3dc_PNT2	*n2;
	p3dc_VRTX	*nV;

	switch (orig->data.t) {
		case P3DC_PNT4:
			n4 = p3dc_new_type(P3DC_PNT4);
			*n4 = *((p3dc_PNT4 *)(orig->data.p));
			return (p3dc_new_node(n4, NULL, P3DC_NODE_FREEPAYLOAD));
		case P3DC_PNT3:
			n3 = p3dc_new_type(P3DC_PNT3);
			*n3 = *((p3dc_PNT3 *)(orig->data.p));
			return (p3dc_new_node(n3, NULL, P3DC_NODE_FREEPAYLOAD));
		case P3DC_PNT2:
			n2 = p3dc_new_type(P3DC_PNT2);
			*n2 = *((p3dc_PNT2 *)(orig->data.p));
			return (p3dc_new_node(n2, NULL, P3DC_NODE_FREEPAYLOAD));
		case P3DC_VRTX:
			nV = p3dc_new_type(P3DC_VRTX);
			*nV = *((p3dc_VRTX *)(orig->data.p));
			return (p3dc_new_node(nV, NULL, P3DC_NODE_FREEPAYLOAD));
		default:
			return NULL;
	}
}


/*
 * This function will take two nodes and compute an intercept between
 * them based on a value t that is between 0 and 1. The result is
 * either a new node or the node 'xres' is reused if it is present.
 */
static p3dc_NODE *
intercept_node(p3dc_NODE *a, p3dc_NODE *b, float t, p3dc_NODE *xres) {
	p3dc_PNT4 *n4, *a4, *b4;
	p3dc_PNT3 *n3, *a3, *b3;
	p3dc_PNT2 *n2, *a2, *b2;
	p3dc_VRTX *nV, *aV, *bV;

	switch(a->data.t) {
		case P3DC_PNT4:
			a4 = (p3dc_PNT4 *)(a->data.p);
			b4 = (p3dc_PNT4 *)(b->data.p);
			n4 = (xres == NULL) ? p3dc_new_type(P3DC_PNT4) : xres->data.p;
			n4->x = a4->x + t * (b4->x - a4->x);
			n4->y = a4->y + t * (b4->y - a4->y);
			n4->z = a4->z + t * (b4->z - a4->z);
			n4->w = a4->w + t * (b4->w - a4->w);
			return ((xres == NULL) ? p3dc_new_node(n4, NULL, P3DC_NODE_FREEPAYLOAD) : xres);
		case P3DC_PNT3:
			a3 = (p3dc_PNT3 *)(a->data.p);
			b3 = (p3dc_PNT3 *)(b->data.p);
			n3 = (xres == NULL) ? p3dc_new_type(P3DC_PNT3) : xres->data.p;
			n3->x = a3->x + t * (b3->x - a3->x);
			n3->y = a3->y + t * (b3->y - a3->y);
			n3->z = a3->z + t * (b3->z - a3->z);
			return ((xres == NULL) ? p3dc_new_node(n3, NULL, P3DC_NODE_FREEPAYLOAD) : xres);
		case P3DC_PNT2:
			a2 = (p3dc_PNT2 *)(a->data.p);
			b2 = (p3dc_PNT2 *)(b->data.p);
			n2 = (xres == NULL) ? p3dc_new_type(P3DC_PNT2) : xres->data.p;
			n2->u = a2->u + t * (b2->u - a2->u);
			n2->v = a2->v + t * (b2->v - a2->v);
			return ((xres == NULL) ? p3dc_new_node(n2, NULL, P3DC_NODE_FREEPAYLOAD) : xres);
		case P3DC_VRTX:
			aV = (p3dc_VRTX *)(a->data.p);
			bV = (p3dc_VRTX *)(b->data.p);
			nV = (xres == NULL) ? p3dc_new_type(P3DC_VRTX) : xres->data.p;
			nV->xid = aV->xid;
			nV->v.x = nV->v.y = nV->v.z = 0;
			nV->tv.x = aV->tv.x + t * (bV->tv.x - aV->tv.x);
			nV->tv.y = aV->tv.y + t * (bV->tv.y - aV->tv.y);
			nV->tv.z = aV->tv.z + t * (bV->tv.z - aV->tv.z);
			nV->tv.w = aV->tv.w + t * (bV->tv.w - aV->tv.w);
			nV->ccode = 0;
			if (nV->tv.x <= -nV->tv.w)
				nV->ccode |= (1 << NEG_X_PLANE);
			if (nV->tv.x >= nV->tv.w)
				nV->ccode |= (1 << POS_X_PLANE);
			if (nV->tv.y <= - nV->tv.w)
				nV->ccode |= (1 << NEG_Y_PLANE);
			if (nV->tv.y >= nV->tv.w)
				nV->ccode |= (1 << POS_Y_PLANE);
			if (nV->tv.z <= 0)
				nV->ccode |= (1 << NEG_Z_PLANE);
			if (nV->tv.z >= nV->tv.w)
				nV->ccode |= (1 << POS_Z_PLANE);
			nV->cid = aV->xid;
			return ((xres == NULL) ? p3dc_new_node(nV, NULL, P3DC_NODE_FREEPAYLOAD) : xres);
		default:
			return NULL;
	}
}


/*
 * Return values from one 'clip':
 *
 * Result 	Vertices	
 * Code	Returned	Description
 * ======	========	=========================================
 *  0		None		A and B are both outside the plane
 *  1		A only		A is on the plane in question (or A == B)
 *  2		A, X		A is inside, X is intercept to B
 *  3		A, B		Both A and B are inside the plane
 *  4		X, B		A is outside but B is inside.
 *  5		B only		B is on the plane in question.
 *
 *
 *Clip Results:
 *
 *	PNT4	A	; a vrtx
 *	PNT4	B	; b vrtx
 *	FLOAT	t	; t value (between 0.0, 1.0)
 *	char	r	; Result code (see above)
 *
 * Clip Parameters:
 *
 *	PNT4	A	; a vrtx
 *	PNT4	B	; b vrtx
 * 	char	plane ; (0 thru 6)
 */
#ifdef CLIPPING_DEBUG
/*
 * Validate returns false if the node is outside the
 * view frustum. Note that being exactly on the planes
 * of the frustum is ok since we clip to the plane
 * exactly.
 */
void
validate(p3dc_NODE *n, int plane) {
	p3dc_VRTX *v = (p3dc_VRTX *)(n->data.p);

	if ((1 << NEG_X_PLANE) & plane)
		assert(v->tv.x >= (-v->tv.w - P3DC_SIGMA));
	if ((1 << POS_X_PLANE) & plane)
		assert(v->tv.x <= (v->tv.w + P3DC_SIGMA));
	if ((1 << NEG_Y_PLANE) & plane)
		assert(v->tv.y >= (-v->tv.w - P3DC_SIGMA));
	if ((1 << POS_Y_PLANE) & plane)
		assert(v->tv.y <= (v->tv.w + P3DC_SIGMA));
	if ((1 << NEG_Z_PLANE) & plane)
		assert(v->tv.z >= (- P3DC_SIGMA));
	if ((1 << POS_Z_PLANE) & plane)
		assert(v->tv.z <= (v->tv.w + P3DC_SIGMA));
	return; 
}
#endif

/* 
 * This code takes two vertices and clips them to the
 * passed in plane. 
 *
 * Two things are depended upon:
 *		1) Outcodes are pre-computed.
 *		2) The vertices have been transformed into clip space.
 *		   (which means we use the tv values in the vertex rather
 *		    than the v values)
 */
static int
clip_it(p3dc_VRTX *vA, p3dc_VRTX *vB, float *t, int plane) {
	int Acode, Bcode;
	p3dc_PNT4	*A, *B;

	A = &(vA->tv);
	B = &(vB->tv);
	Acode = vA->ccode & (1<<plane);
	Bcode = vB->ccode & (1<<plane);

	if (Acode && Bcode)
		return CLIP_NONE;
	if (!(Acode || Bcode))
 		return CLIP_A_B;

	switch (plane) {
		case NEG_X_PLANE: 
			*t = (A->x + A->w) / -((B->w - A->w) + (B->x - A->x));
			break;
		case POS_X_PLANE: 
			*t = (A->x - A->w) / ((B->w - A->w) - (B->x - A->x));
			break;
		case NEG_Y_PLANE: 
			*t = (A->y + A->w) /-((B->w - A->w) + (B->y - A->y));
			break;
		case POS_Y_PLANE: 
			*t = (A->y - A->w) / ((B->w - A->w) - (B->y - A->y));
			break;
		case NEG_Z_PLANE:  /* z == 0 */
			*t = -A->z / (B->z - A->z);
			break;
		case POS_Z_PLANE: 
			*t = (A->z - A->w) / ((B->w - A->w) - (B->z - A->z));
			break;
	}

	/* Things that should be true at this point ...
	 * - A or B (BUT NOT BOTH!) is inside the clipping plane
	 * - t must be between 0 and 1 (ie there must be a plane intercept)
 	 */
	return (Acode) ? CLIP_X_B : CLIP_A_X ;
}

/*
 * Once clipped we "deal" with the result which can be one of
 * four things:
 *		1) Do nothing (both nodes are clipped)
 *		2) Output the current node as its inside the clip region
 *		3) Output the current node and an intercept node.
 *		4) Add just the intercept node.
 */
static int
operate_node(int code, 
	p3dc_LIST *src, p3dc_LIST *dst, 
	p3dc_NODE *a, p3dc_NODE *b, 
	p3dc_FLOAT dt, int copy_it) 
{
	p3dc_NODE *nX;

	switch (code) {
		case CLIP_NONE:
		case CLIP_B_ONLY:
			/* 
			 * If A doesn't appear in the final list, just free
			 * it (assuming this isn't the polygon's list)
			 */
			if ((! copy_it) && (a != src->head)) {
				p3dc_remove_node(src, P3DC_NODE_THIS, a);
				p3dc_free_node(a);
			}
			return 0; /* no new nodes are added */

		case CLIP_A_ONLY:
		case CLIP_A_B:
			/*
			 * Move the initial node to the result list.
			 */
			if (copy_it) {
				nX = copy_node(a);
			} else {
				p3dc_remove_node(src, P3DC_NODE_THIS, a);
				nX = a;
			}
			p3dc_add_node(dst, nX, P3DC_LIST_TAIL);
			return 1;
		case CLIP_A_X: 
			/*
			 * Move the initial node to the result list and
			 * add a new intercept node
			 */
			if (copy_it) {
				nX = copy_node(a);
			} else {
				p3dc_remove_node(src, P3DC_NODE_THIS, a);
				nX = a;
			}
			p3dc_add_node(dst, nX, P3DC_LIST_TAIL);
			/*
			 * Add the intercept node to the outgoing list.
			 */
			nX = intercept_node(a, b, dt, NULL);
			p3dc_add_node(dst, nX, P3DC_LIST_TAIL);
			return 2;

		case CLIP_X_B:
			/*
			 * Re-use the node in A, UNLESS it is the FIRST
			 * node, and make it part of the outgoing
			 * List
			 */
			if (copy_it || (src->head == a))
				nX = intercept_node(a, b, dt, NULL);
			else {
				p3dc_remove_node(src, P3DC_NODE_THIS, a);
				nX = intercept_node(a, b, dt, a);
			}
			p3dc_add_node(dst, nX, P3DC_LIST_TAIL);
			return 1;
		default:
			return 0;
	}
}

/*
 * This is the primary clipping code.
 *
 * The code assumes that lists[0] is the vertex list for a convex
 * polygon. The parameter 'nLists' contains the number of lists to
 * clip which can be a few as 1 (flat shaded polygons) and as many
 * as 5 (polygon, vertex lights, three textures (lightmap, decal, detail)
 *
 * When the flag 'make_copy' is non-zero it causes the code to copy a
 * list while it clips it. This function gets called for each plane
 * to clip against, usually one or two times.
 */
static void
clip_contexts(CLIP_CONTEXT lists[], int plane, int make_copy) {
	p3dc_FLOAT dt;
	int i, code = 0;
	int loopcnt;

	/*
	 * DANGER DANGER - operate_node manipulates the list so
	 * we have to be careful walking it. In general the current
	 * node may be moved to the new list so it's next point is
	 * unreliable after we call operate_node. Therefore we stash
	 * a copy in NEXT, and then clip the segment.
	 */
	for (i = 0; i < 5; i++) {
		if (lists[i].src == NULL) 
			continue;
		lists[i].current = lists[i].first = lists[i].src->head;
		lists[i].dstNodes = 0;
	}

	loopcnt = 0;
	while (lists[0].current != NULL) {
		loopcnt++;
		assert(loopcnt < 1000);
		for (i = 0; i < 5; i++) {
			p3dc_NODE *next;
			if (lists[i].src == NULL)
				continue;
			next = lists[i].next = lists[i].current->nxt;
			if (lists[i].next == NULL)
				lists[i].next = lists[i].first;
			if (i == 0)
				code = clip_it((p3dc_VRTX *)(lists[i].current->data.p), 
					       (p3dc_VRTX *)(lists[i].next->data.p), &dt, plane);

			/*
			 * If i == 0, then actually do the clipping for the vertices.
			 * Otherwise we reuse the delta we got from the vertices to
			 * clip the other lists.
			 */
			lists[i].dstNodes += operate_node(code, lists[i].src, 
										lists[i].dst, lists[i].current, 
										lists[i].next, dt, make_copy);
			lists[i].current = next;
		}
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
	B A S I C   C L I P P I N G   F U N C T I O N S


  These routines provide clipping in 3D for all primitives (points,
  lines, and polygons. The functions return 0 if the primitive is
  drawable and non-zero if the item is not visible.

  The exception is clip_face which returns a new face structure that
  is clipped on success, otherwise it returns NULL if the face is
  completely off screen.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*
 * clip point - This function computes the clip code (a bitmask of the
 * planes that a vertex is outside of) and stores it in the vertex structure.
 * It can be used standalone to return a boolean for whether or not a point
 * should be clipped, or to simply initialize the clip codes for a vertex.
 *
 */
int
p3dc_clip_point(p3dc_POINT *point) {
	return (point->p0->ccode == 0) ? CLIP_A_B : CLIP_NONE;
}

/*
 * This function takes a p3dc_LINE that has been transformed to the current
 * view, and determines if its endpoints are on the screen.
 *
 * It returns a clip code (CLIP_xxx) and two deltas indicating a 't' value
 * to adjust the endpoints to be in the view. It is important to note that
 * the delta values assume the Original endpoints!
 *
 * Also note that it clips lines that have a single point on the screen.
 */
int
p3dc_clip_line(p3dc_LINE *ll, float *delta1, float *delta2) {
	int plane;
	int ocode;
	int accept, reject;
	int bChg = 0, aChg = 0;
	float dt;

	/* Short cut if we've clipped this line already. */
	if (ll->cid == ll->p1->xid)
			return ll->ccode;

	/* Trivial acceptance/rejection test */
	accept = reject = (ll->p1->ccode);
	ocode  = (ll->p2->ccode);
	accept |= ocode;
	reject &= ocode;

	/* Set up the initial clip data */
	ll->cid = ll->p1->xid;
	ll->ccode = CLIP_NONE; /* always safe (doesn't draw the line!) */
	ll->c1 = (ll->p1)->tv;
	ll->c2 = (ll->p2)->tv;

	/* Trivial acceptance test, both A and B are inside the frustum. */
	if (accept == 0) {
		ll->ccode = CLIP_A_B;
		return CLIP_A_B;
	}

	/* 
	 * Trivial rejection, both A and B are outside at least one of the
	 * frustum's planes.
	 */
	if (reject != 0) {
		ll->ccode = CLIP_NONE;
		return CLIP_NONE;
	}

	/* Initialize the deltas */
	*delta1 = 0;
	*delta2 = 1;
	for (plane = 0; plane < 6; plane++) {
		/* Only clip on those planes that the line is outside of */
		if ((accept & (1 << plane)) == 0)
			continue;
		/* 
		 * Clip against a plane and figure out if delta 1 or delta 2
		 * should be updated.
		 */
		switch (clip_it(ll->p1, ll->p2, &dt, plane)) {
			case CLIP_NONE:
				/* Both vertices are outside the plane, don't draw it */
				ll->ccode = CLIP_NONE;
				return (CLIP_NONE);
			case CLIP_A_ONLY:
				/* A == edge, B off screen, degenerate case */
				ll->ccode = CLIP_A_ONLY;
				ll->c2 = ll->c1;
				return (CLIP_A_ONLY);
			case CLIP_A_X:
				/* assert(dt >= 0); this is always true. */
				if (*delta2 > dt) {
					*delta2 = dt;
					bChg++;
				}
				break;
			case CLIP_A_B:
				break;
			case CLIP_X_B:
				/* assert(dt >= 0); This is always true */
				if (*delta1 < dt) {
					*delta1 = dt;
					aChg++;
				}
				break;
			case CLIP_B_ONLY:
				/* B == edge, A off screen, degnerate case */
				ll->ccode = CLIP_B_ONLY;
				ll->c1 = ll->c2;
				return (CLIP_B_ONLY);
			default:
				assert(0);
				break;
		}
	}
	if (*delta1 > *delta2) {
		ll->ccode = CLIP_NONE;
		return CLIP_NONE;
	}

	/* Well one of them has to change, so scale it's clip point and return */
	if (aChg)
		interceptP4(&(ll->p1->tv), &(ll->p2->tv), *delta1, &(ll->c1));
	if (bChg)
		interceptP4(&(ll->p1->tv), &(ll->p2->tv), *delta2, &(ll->c2));

	if (aChg && bChg) {
		ll->ccode = CLIP_X_X;
		return CLIP_X_X;
	} else if (aChg) {
		ll->ccode = CLIP_X_B;
		return CLIP_X_B;
	} else if (bChg) {
		ll->ccode = CLIP_A_X;
		return CLIP_A_X;
	}
	ll->ccode = CLIP_NONE;
	return (CLIP_NONE);
}

void
verify_contexts(CLIP_CONTEXT ctx[]) {
	int i;
	for (i = 0; i < 5; i++) {
		if (ctx[i].src != NULL) {
			p3dc_NODE *n;
			for (n = FIRST_NODE(ctx[i].src); n != NULL; n = NEXT_NODE(ctx[i].src))
				assert(n->owner == ctx[i].src);
		}
	}
}

/*
 * This code is used for both clipping polygons and faces. 
 *
 * The code takes as arguments an array of list pointers, this
 * array points to the following lists:
 *		0 - The vertices of the polygon/face
 *		1 - The "light" intensities at each vertex
 *		2 - Co-ordinates of the base texture.
 *		3 - Co-ordinates for each vertex in a lightmap texture
 *		4 - Co-ordinates for each vertex in a detail texture.
 *
 * Only the first list is actually "clipped" the other lists are 
 * made to "follow along" by inserting nodes in them that represent
 * the linearly interoplated point between nodes. 
 *
 * List 0 must always be non-null, the others may be NULL if there
 * are no nodes of that type to clip.
 */
int
p3dc_clip_lists(p3dc_LIST *(f_src[]), p3dc_LIST *(f_clip[])) {
	p3dc_NODE *t;
	CLIP_CONTEXT clip_lists[5];
	int i, res;
	int	out_code, accept = 0, reject = 0xff;
	int plane, did_clip = 0;

	for (t = FIRST_NODE(f_src[0]); t != NULL; t = NEXT_NODE(f_src[0])) {
		out_code = ((p3dc_VRTX *)(t->data.p))->ccode;
		accept |= out_code;
		reject &= out_code;
	}

	if (accept == 0)
		return CLIP_A_B;

	if (reject != 0)
		return CLIP_NONE;

	res = CLIP_NONE; /* a safe default */

	/*
	 * Set up the lists we'll be clipping. The interesting bit here
	 * is that we only _clip_ the vertices, then with the results
	 * we adjust the other lists appropriately.
	 */
	for (i = 0; i < 5; i++) {
		clip_lists[i].src = f_src[i];
		if (f_src[i] != NULL)
			clip_lists[i].dst = p3dc_new_list(f_src[i]->n_type, P3DC_LIST_LINKED);
		else
			clip_lists[i].dst = NULL;
		clip_lists[i].active = (f_src[i] != NULL);
	}

	for (plane = 0; (plane < 6) && (accept != 0); plane++) {
		int mask;

		verify_contexts(clip_lists);

		mask = 1 << plane;
		/* Only do the clipping if there are nodes outside this plane */
		if ((accept & mask) != 0) {
			/* 
			 * Actually do the clipping, force a copy if the source
			 * list is in fact the original source list
			 */
			clip_contexts(clip_lists, plane, clip_lists[0].src == f_src[0]);

			/* note that clipping did occur */
			did_clip++;

			/*
			 *  If we clipped all the vertices, leaving us only
		 	 * an empty list, then this polygon is completely off
			 * the screen so clean up and return CLIP_NONE.
			 */
			if (clip_lists[0].dst->tail == NULL) {
				for (i = 0; i < 5; i++) {
					if (clip_lists[i].src != f_src[i]) {
						p3dc_free_list(clip_lists[i].src);
						p3dc_free_type(clip_lists[i].src);
					}
					if (clip_lists[i].dst != NULL) {
						p3dc_free_list(clip_lists[i].dst);
						p3dc_free_type(clip_lists[i].dst);
					}
				}
				return CLIP_NONE;
			}
			accept = (accept & ~mask); /* turn off this plane */

			/*
			 * The flag 'done' will be set if the polygon
			 * is now clipped to completely inside the 
			 * view frustum.
			 */
			if (accept != 0) {
				/* 
			 	 * if we weren't finished (polygon intercepts more
				 * than one plane) then setup the lists for the next
			 	 * plane. 
			 	 */
				for (i = 0; i < 5; i++) {
					p3dc_LIST *old_src;
					/*
					 * Very weird case I've not optimized out. If :
					 *   -- The source list is non-null AND
					 *   -- The list is non-empty AND
					 *	 -- The list isn't the original source THEN
					 *
					 * When the FIRST node in the list gets dropped, 
					 * because it is outside the plane we just clipped
					 * we don't reuse it because we need it for the last
					 * test (last node to first node) so instead it gets
					 * copied to the destination list and the one left
					 * on the source list is superfluous.
					 */
					if ((clip_lists[i].src != NULL) && 
						(clip_lists[i].src->head != 0) &&
						(clip_lists[i].src != f_src[i])) {
						p3dc_NODE *nn;

						/* Must only be one node left!! */
						assert(clip_lists[i].src->head == 
								clip_lists[i].src->tail);
						nn = p3dc_remove_node(clip_lists[i].src, 
															P3DC_NODE_FIRST);
						p3dc_free_node(nn); 
					}

					/* Now swap source and destination. If source isn't
					 * the original source, its a copy so re-use the list
					 * struct, otherwise allocate a new list structure
					 * for the destination.
					 */
					if ((clip_lists[i].src == NULL) || (clip_lists[i].src != f_src[i]))
						old_src = clip_lists[i].src;
					else 
						old_src = p3dc_new_list(clip_lists[i].dst->n_type, P3DC_LIST_LINKED);

					/* 
					 * Make the destination list the new source.
					 * This works in all cases because even if this list
					 * isn't active we just assign NULL to NULL.
					 */
					clip_lists[i].src = clip_lists[i].dst;
					clip_lists[i].dst = old_src;
					/* Transfer the node count along as well */
					clip_lists[i].srcNodes = clip_lists[i].dstNodes;
				}
			}
		}
	}

	/*
	 * At this point we are done clipping and its time to return the
	 * result. The result is sitting in the dst pointers of clip lists
	 */
	for (i = 0; i < 5; i++) {
		if (f_clip[i] != NULL) {
			p3dc_free_list(f_clip[i]);
			p3dc_free_type(f_clip[i]);
		}
		if (f_src[i] != NULL)
			f_clip[i] = clip_lists[i].dst;
		else
			f_clip[i] = NULL;
		if (clip_lists[i].src != f_src[i]) {
			p3dc_free_list(clip_lists[i].src);
			p3dc_free_type(clip_lists[i].src);
		}
	}
	/* 
	 * if somehow we never called clip_contexts we return CLIP_A_B 
	 * XXX that should be a bug! XXX
	 */
	return (did_clip) ? CLIP_X_X : CLIP_A_B;
}

