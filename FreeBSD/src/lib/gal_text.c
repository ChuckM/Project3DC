/*
 * gal_text.c - Stub driver for the UNIX version.
 * $Id: gal_text.c,v 1.2 1999/08/16 09:51:24 cmcmanis Exp cmcmanis $
 *
 * This is the graphics abstraction layer driver for the VooDoo
 * graphics card.
 *
 * Change Log:
 *	$Log: gal_text.c,v $
 *	Revision 1.2  1999/08/16 09:51:24  cmcmanis
 *	Woo-hoo. Textures work again.
 *
 *	Revision 1.1  1999/08/14 23:18:56  cmcmanis
 *	Initial revision
 *
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
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "p3dc.h"

#ifdef u_long
#undef u_long
#endif

typedef unsigned long u_long;

/*
 * We render into this "psuedo" frame buffer.
 */
static u_long *fb;	/* Frame buffer */
static u_long *db;	/* Depth buffer */

struct vtx {
	float	x,y;	/* 2D co-ordinates */
	float	u,v;	/* Texture co-ordinates */
	int 	ndx;
	long 	w;
	long	color;
};

/*
 * This is where it is very important to "snap" the vertices.
 */
static void
cvt_vertex(
	struct vtx *v,		/* Vertex for the data */
	p3dc_VIEW *vp,		/* Camera (for scaling co-ordinates) */
	p3dc_VRTX *pq,		/* Transformed Vertex */
	p3dc_CLR *clr,		/* Base color of the Vertex */
	p3dc_PNT4 *light,	/* Optional light modifier (fractionates color) */
	p3dc_PNT2 *texture) /* Optional texture co-ordinates associated with v */
{
	float oow;
	unsigned char r, g, b;

	if (clr == NULL) clr = &COLOR_white; /* default color */
	oow = (float) (1.0f/pq->tv.w);
	v->w = (long) (pq->tv.w);

#ifdef VERIFY_CLIP
	/* While debugging, enforce clipping */
	assert(fabs(pq->tv.x * oow)<= 1.0001f);
	assert(fabs(pq->tv.y * oow)<= 1.0001f);
#endif

	v->x = floorf(pq->tv.x * oow * vp->scale_x + vp->center_x + 0.5); 
	v->y = floorf(pq->tv.y * oow * vp->scale_y + vp->center_y + 0.5);
	if (light) {
		r = (unsigned char) (light->x * (float) (clr->gRED));
		g = (unsigned char) (light->y * (float) (clr->gGREEN));
		b = (unsigned char) (light->z * (float) (clr->gBLUE));
	} else {
		r = clr->gRED;
		g = clr->gGREEN;
		b = clr->gBLUE;
	}
	if (texture) {
		v->u = texture->u;
		v->v = texture->v;
	} else {
		v->u = 0;
		v->v = 0;
	}
	v->color = (r << 16) | (g << 8) | b;
	v->ndx = (int) (v->y * 640) + (int)(v->x);
}

#define MAX_INDEX 307200

/*
 * Draw pixel into the frame buffer
 */
static void
wpixel(struct vtx *v) {
	if ((v->ndx < 0) || (v->ndx > 307199))
			return;
	*(fb+v->ndx) = v->color;
	*(db+v->ndx) = v->w;
}

/*
 * Draw a line of pixels into the frame buffer.
 */
static void
wline(struct vtx *v1, struct vtx *v2) {
	double dx, dy;
	double ix, iy;
	float r1, r2, g1, g2, b1, b2;
	float ir, ig, ib;
	int r, g, b;
	int	length;
	struct vtx v;
	
	dx = v2->x - v1->x;
	dy = v2->y - v1->y;
	r1 = (float) ((v1->color >> 16) & 0xff);
	r2 = (float) ((v2->color >> 16) & 0xff);
	g1 = (float) ((v1->color >> 8) & 0xff);
	g2 = (float) ((v2->color >> 8) & 0xff);
	b1 = (float) ((v1->color ) & 0xff);
	b2 = (float) ((v2->color ) & 0xff);
	length = (int)((fabs(dx) > fabs(dy)) ? fabs(dx) : fabs(dy));
	if (EQZ(length)) {
		wpixel(v1);
		return;
	}
	ir = (r2 - r1)/length;
	ig = (g2 - g1)/length;
	ib = (b2 - b1)/length;
#if 0
	p3dc_log("LINE: Color delta : red (%f) => (%f)  delta %f\n",
			r1, r2, ir);
	p3dc_log("LINE: Color delta : green (%f) => (%f)  delta %f\n",
			g1, g2, ig);
	p3dc_log("LINE: Color delta : blue (%f) => (%f)  delta %f\n",
			b1, b2, ib);
#endif
	ix = dx/length;
	iy = dy/length;
	v.x = v1->x;
	v.y = v1->y;
	while (--length) {
		float x, y;
		r = (int)(floorf(r1+0.5));
		g = (int)(floorf(g1+0.5));
		b = (int)(floorf(b1+0.5));
		x = floorf(v.x + 0.5);
		y = floorf(v.y + 0.5);
		v.ndx = (int)( x + y * 640);
		v.color = r<<16 | g<<8 | b;
		wpixel(&v);
		v.x += ix;
		v.y += iy;
		r1 += ir;
		b1 += ib;
		g1 += ig;
	}
}

/*
 * Scan convert a polygon into the frame buffer.
 */
static void
wpoly(int in, struct vtx *vl) {
	int i;
	/* XXX convert to filled polygons */
	for (i = 0; i < (in-1); i++) {
		wline(vl+i, vl+1+i);
	}
	wline(vl+in-1, vl);
}

/*
 * Draw a point on the screen using the grDrawPoint primitive.
 * This is computationally pretty expensive, we should access the
 * frame buffer directly, but since I don't draw that many points
 * I've not taken the time to optimize this.
 */
void
__hw_draw_point(p3dc_VIEW *vp, p3dc_POINT *p) { 
	struct vtx v;
	cvt_vertex(&v, vp, p->p0, p->color, &(p->light), NULL);
	wpixel(&v);
}

/*
 * Draw a line in the frame buffer. The vertices are assumed to be in
 * world co-ordinates and are transformed into view co-ordinates using
 * the P matrix in the camera.
 */
void
__hw_draw_line(p3dc_VIEW *vp, p3dc_LINE *ll) {
	struct vtx p1, p2;
	cvt_vertex(&p1, vp, ll->p1, ll->color, &(ll->light), NULL);
	cvt_vertex(&p2, vp, ll->p2, ll->color, &(ll->light), NULL);
	wline(&p1, &p2);
}

static struct vtx *vtxArray;
static int vtxSize;

int __p3dc_debug_trigger = 0;

/*
 * Draw a polygon with flat shading.
 *
 * This code trys to be a bit tricky in the following way:
 * The vtxArray is dynamically sized to hold the largest
 * polygon we will draw. However we don't know ahead of time
 * what that size will be. Thus when we traverse the list
 * of vertices, making GrVertex structures, if we find we
 * don't have enough array slots we stop, allocate an array
 * that is 10 slots larger, and start over. This means that
 * in the first few frames of rendering we will pay a price
 * for the reallocation, however after a while it will be
 * big enough to hold all polygons.
 */
void
__hw_draw_polygon(p3dc_VIEW *vp, p3dc_POLY *pp, p3dc_LIST *vL) {
	p3dc_NODE *t;
	int n;
	p3dc_PNT3 p1, p2, p3;
	p3dc_FLOAT oow, c;
	p3dc_VRTX *vv;

	while (1) {
		for (t = vL->head, n = 0; t != NULL; t = t->nxt, n++) {
			if (n == vtxSize) {
				if (vtxArray) {
					free(vtxArray);
				}
				vtxArray = (struct vtx *)malloc((vtxSize+10)*sizeof(struct vtx));
				vtxSize += 10;
				break;
			}

			/*
			 * Process the vertices to see if we can cull the polygon
			 * (and thus not draw it)
			 */
			if (n < 3) {
				vv = (p3dc_VRTX *)(t->data.p);
				oow = (1.0f / vv->tv.w);
				switch (n) {
					case 0:
						p1.x = vv->tv.x * oow;
						p1.y = vv->tv.y * oow;
						p1.z = vv->tv.z * oow;
						break;
					case 1:
						p2.x = vv->tv.x * oow;
						p2.y = vv->tv.y * oow;
						p2.z = vv->tv.z * oow;
						break;
					case 2:
						p3.x = vv->tv.x * oow;
						p3.y = vv->tv.y * oow;
						p3.z = vv->tv.z * oow;
						c = (p3.x * ((p1.z * p2.y) - (p1.y * p2.z))+
							 p3.y * ((p1.x * p2.z) - (p1.z * p2.x))+
							 p3.z * ((p1.y * p2.x) - (p1.x * p2.y)));
						if (c > 0) 
							return;
				}
			}
				
			cvt_vertex(vtxArray+n, vp, (p3dc_VRTX *)(t->data.p), pp->color,
														   &(pp->light), NULL);
		}
		if (t == NULL) {
			wpoly(n, vtxArray);
			return;
		}
	}
}
void __hw_enable_texture(void *texture) {
}

void __hw_disable_texture() {
}


/*
 * Draw a face with gourard shading.
 *
 * This code trys to be a bit tricky in the following way:
 * The vtxArray is dynamically sized to hold the largest
 * polygon we will draw. However we don't know ahead of time
 * what that size will be. Thus when we traverse the list
 * of vertices, making GrVertex structures, if we find we
 * don't have enough array slots we stop, allocate an array
 * that is 10 slots larger, and start over. This means that
 * in the first few frames of rendering we will pay a price
 * for the reallocation, however after a while it will be
 * big enough to hold all polygons.
 */
void
__hw_draw_face(p3dc_VIEW *vp, p3dc_FACE *ff, p3dc_LIST *(lists[]), void *txt, int cull) {
	p3dc_NODE *t_vertex = NULL;
	p3dc_NODE *t_light = NULL;
	p3dc_NODE *t_texture = NULL;
	p3dc_LIST *vertex_list = lists[0]; /* The vertices of the face */
	p3dc_LIST *light_list = lists[1]; /* Potentially an array of lights */
	p3dc_LIST *texture_list = lists[2]; /* Potentially a decal texture map */
	p3dc_POLY *pp = &(ff->poly);
	p3dc_PNT3 p1, p2, p3;
	int num_verts;

	cull = 0;
	while (1) {
		t_light = (light_list != NULL) ? light_list->head : NULL;
		t_vertex = (vertex_list != NULL) ? vertex_list->head : NULL;
		t_texture = (texture_list != NULL) ? texture_list->head : NULL;
		num_verts = 0;
		while (t_vertex != NULL) {
			if (num_verts == vtxSize) {
				if (vtxArray) {
					free(vtxArray);
				}
				vtxArray = (struct vtx *)malloc((vtxSize+10)*sizeof(struct vtx));
				assert(vtxArray != NULL);
				vtxSize += 10;
				break; /* Jump out of the inner loop and restart! */
			}

			/*
			 * Process the vertices to see if we can cull the face
			 * (and thus not draw it)
			 */
			if (num_verts < 3) {
				p3dc_VRTX *vv;
				p3dc_FLOAT oow, c;

				vv = (p3dc_VRTX *)(t_vertex->data.p);
				oow = (1.0f / vv->tv.w);
				switch (num_verts) {
					case 0:
						p1.x = vv->tv.x * oow;
						p1.y = vv->tv.y * oow;
						p1.z = vv->tv.z * oow;
						break;
					case 1:
						p2.x = vv->tv.x * oow;
						p2.y = vv->tv.y * oow;
						p2.z = vv->tv.z * oow;
						break;
					case 2:
						p3.x = vv->tv.x * oow;
						p3.y = vv->tv.y * oow;
						p3.z = vv->tv.z * oow;
						c = (p3.x * ((p1.z * p2.y) - (p1.y * p2.z))+
							 p3.y * ((p1.x * p2.z) - (p1.z * p2.x))+
							 p3.z * ((p1.y * p2.x) - (p1.x * p2.y)));
						if (cull && c > 0)
							return;
				}
			}
				
			cvt_vertex(vtxArray+num_verts, vp, 
					(p3dc_VRTX *)(t_vertex->data.p), 
					pp->color,
					(t_light != NULL) ? (p3dc_PNT4 *) t_light->data.p : NULL,
					(t_texture != NULL) ? (p3dc_PNT2 *) t_texture->data.p : NULL);
			t_vertex = t_vertex->nxt;
			if (t_light)
				t_light = t_light->nxt;
			if (t_texture)
				t_texture = t_texture->nxt;
			num_verts++;
		}

		if ((t_vertex != NULL) && (vtxArray == NULL)) 
			return; /* allocation failed */

		if (t_vertex != NULL)
			continue; /* Else we are a restart so keep going */
		if (txt == NULL) {
			__hw_disable_texture();
		} else {
			__hw_enable_texture(txt);
		}
		switch (num_verts) {
			case 0:
				return;
			case 1:
				wpixel(vtxArray);
				return;
			case 2:
				wline(vtxArray, vtxArray+1);
				return;
			default:
				wpoly(num_verts, vtxArray);
				return;
		}
	}
}


/*
 * Initialize GAL layer.
 */
int
__hw_init() {
	fb = (u_long *)malloc(640 * 480 * sizeof(u_long));
	db = (u_long *)malloc(640 * 480 * sizeof(u_long));
	if ((fb != NULL) && (db != NULL))
		return 0;
	return 1;
}

void
__hw_effect_fog(p3dc_CLR *fog) {
}

/*
 * prepare frame for rendering.
 */
void
__hw_prepare_frame(p3dc_CLR *bg) {
	memset(fb, ((bg->rgba)>>8) & 0xfff, 640*480*sizeof(u_long));
	memset(db, -1, 640*480*sizeof(u_long));
}

void
__hw_clear_region(p3dc_CLR *cl, float x, float y, float w, float h) {
	int i, j;
	int x1, y1, x2, y2;

	x1 = (int) x;
	x2 = (int) x + (int) w;
	y1 = (int) y;
	y2 = (int) y + (int) h;

	for (i = y1; i < y2; i++) {
		int ndx = y1 * 640;
		for (j = x1; j < x2; j++) {
			*(fb+ndx+j) = (u_long)((cl->rgba >> 8) & 0xfff);
			*(db+ndx+j) = 0xffffffff;
		}
	}
}

void
__hw_show_frame(int wait) {
}

/*
 * Close glide.
 */
void
__hw_shutdown() {
	if (fb)
		free(fb);
	if (db)
		free(db);
	if (vtxArray)
		free(vtxArray);
}

int
__hw_control(int func, va_list vlst) {
	int	*p1, *p2;
	int	a;
	int res = 0;

	switch (func) {
	case P3DC_CTRL_GETRESOLUTION:
		p1 = va_arg(vlst, int *);
		p2 = va_arg(vlst, int *);
		*p2 = 480;
		*p1 = 640;
		break;
	case P3DC_CTRL_DISPLAY:
		a = va_arg(vlst, int);
		switch (a) {
		case P3DC_DISPLAY_OFF:
			break;
		case P3DC_DISPLAY_ON:
			break;
		case P3DC_DISPLAY_MOVE:
			break;
		case P3DC_DISPLAY_RESIZE:
			break;
		default:
			res = 1;
		}
		break;
	case P3DC_CTRL_DEPTHBUFFER:
		a = va_arg(vlst, int);
		break;
	case P3DC_CTRL_GSHADING:
		a = va_arg(vlst, int);
		break;
	case P3DC_CTRL_TRANSPARENT:
		a = va_arg(vlst, int);
		break;
	case P3DC_CTRL_LIGHTMAP:
		a = va_arg(vlst, int);
		break;
	case P3DC_CTRL_OPAQUE:
		break;
	case P3DC_CTRL_DEPTHBIAS:
		break;
	default:
		res = 1;
	}
	return res;
}
 
#ifdef PNGDEBUG

static unsigned char
getred(int r, int c) {
	return ((unsigned char)(r >> 5) << 3);
}

static unsigned char
getgreen(int r, int c) {
	return ((unsigned char)(c >> 3) << 3);
}

static unsigned char
getblue(int r, int c) {
	return ((unsigned char)(((r >> 3) | (c >> 3)) << 3));
}

/*
 * to debug the PNG stuff, this function returns a test
 * pattern as its data.
 */
int
__hw_get_row(int row, unsigned char data[]) {
	int j;
	/* unsigned char r, g, b; */

	for (j = 0; j < 640; j++) {
		data[j*3] = getred(row, j);
		data[j*3+1] = getgreen(row, j);
		data[j*3+2] = getblue(row, j);
	}
	return 0;
}

#else
/*
 * Return a row from our ertwhile frame buffer.
 */
int
__hw_get_row(int row, unsigned char data[]) {
	unsigned long *pixels;
	int i;

	pixels = fb + 1 + (479-row) * 640;

	for (i = 0; i < 640; i ++) {
		u_long pd = *(pixels + i);
		data[i*3+0] = (unsigned char)((pd >> 16) & 0xff);
		data[i*3+1] = (unsigned char)((pd >> 8) & 0xff);
		data[i*3+2] = (unsigned char)(pd & 0xff);
	}
	return 0;
}
#endif

void * 
__hw_load_texture(char *name, int a, int wrap) {
	return NULL;
}

void
__hw_unload_texture(void *txt) {
}
