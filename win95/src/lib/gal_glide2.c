/*
 * gal_glide2.c - This is the driver for the 3dfx GLIDE 2.x library. 
 * $Id: gal_glide2.c,v 1.2 1999-10-31 11:24:11-08 cmcmanis Exp cmcmanis $
 *
 * This is the graphics abstraction layer driver for the VooDoo
 * graphics card.
 *
 * Change Log:
 *	$Log: gal_glide2.c,v $
 *	Revision 1.2  1999-10-31 11:24:11-08  cmcmanis
 *	Camera rewrite is nearly complete. This seems like a logical
 *	milestone
 *
 *	Revision 1.1  1999/08/18 01:37:31  cmcmanis
 *	Initial revision
 *
 *	Revision 1.1  1999/08/18 01:06:01  cmcmanis
 *	Initial revision
 *
 *	Revision 1.13  1999/08/16 09:51:23  cmcmanis
 *	Woo-hoo. Textures work again.
 *
 *	Revision 1.12  1999/08/14 23:18:37  cmcmanis
 *	This version now includes a new model structure that allows
 *	for dynamic changing of colors and textures in the model
 *	after it has been loaded.
 *
 *	Revision 1.11  1999/07/26 03:10:25  cmcmanis
 *	Renaming is complete, ModelTest runs again, the view was added to support Models sharing the same draw functions as other programs.
 *
 *	Revision 1.10  1999/07/25 09:11:50  cmcmanis
 *	The great renaming from g3d/G3D -> p3dc/P3DC
 *
 *	Revision 1.9  1998/11/16 03:40:52  cmcmanis
 *	Updated clipping
 *
 *	Revision 1.9  1998/11/08 05:19:13  cmcmanis
 *	Outdated (renamed to 'H:\P3DC-RCS\src\lib\gal_glide2.c,v').
 *
 *	Revision 1.8  1998/11/06 18:02:06  cmcmanis
 *	Clipping for faces now works as expected.
 *
 *	Revision 1.7  1998/11/05 20:23:38  cmcmanis
 *	Checkin prior to moving into a better source tree structure
 *
 *	Revision 1.6  1998/10/10 07:00:34  cmcmanis
 *	Model parsing is written (not fully debugged) and the texture structures
 *	are closer to their final form. By the next check in we should have a
 *	few simple models up and running with textured models to follow.
 *
 *	Revision 1.5  1998/09/23 08:48:48  cmcmanis
 *	Clipping works! Polygon and line clipping is pretty robust at this point
 *	still looking for corner cases but I believe all of the code is correct at
 *	this point.
 *
 *	Revision 1.4  1998/09/20 08:10:51  cmcmanis
 *	Major rewrite, added list stuff, and primitives. Almost completely
 *	back to where version 4 was at this point. This version is ALPHA0_2
 *
 *	Revision 1.3  1998/09/18 08:34:39  cmcmanis
 *	This update has new clipping code for polygons.
 *	In particular the nodes are not needlessly copied.
 *
 *	Adds the new list capability in util.c/memory.c that
 *	uses a p3dc_NLST (type P3DC_LIST) as a simple
 *	linked list.
 *
 *	Revision 1.2  1998/09/08 04:04:57  cmcmanis
 *	Clipping for lines and points is now verified, allocation
 *	of vertices in the GAL layer is fixed.
 *
 *	Revision 1.1  1998/09/02 08:48:21  cmcmanis
 *	Initial revision
 *
 *	Revision 1.1  1998/08/30 09:46:48  cmcmanis
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
#include <stdarg.h>
#include <assert.h>
#include <math.h>
#include <windows.h>
#include <winbase.h>
#include <glide.h>
#include "p3dc.h"

static GrHwConfiguration __p3dc_glide_hw;


int __p3dc_gal_glide_is_open = 0;

static LARGE_INTEGER ticks_per_count;

// #define snap(x) (float)(floor(x) / 16.0f)

#define snap(f) ((float)(((long)((f) * 16) / 16.0)))
#define SNAP_CONSTANT (3L << 18)
static const float snap_constant = (float) (3 << 18);

static GrFog_t fog[GR_FOG_TABLE_SIZE];

// #define SLOW_SNAP
#define setcolor(vrtx, clr) { (vrtx)->r = clr->gRED;\
							  (vrtx)->g = clr->gGREEN; \
							  (vrtx)->b = clr->gBLUE; \
							  (vrtx)->a = clr->gALPHA; }
int __do_g_shading = 1;

/*
 * Create a GlideVertex structure from our generic structure. We presume
 * the vector has been transformed into view space with a valid w component
 * which we use for 'oow'.
 * 
 * This is where it is very important to "snap" the vertices.
 */
static void
makeGlideVertex(
	GrVertex *g,		/* GrVertex structure */
	p3dc_VIEW *vp,		/* Camera (for scaling co-ordinates) */
	p3dc_VRTX *pq,		/* Transformed Vertex */
	p3dc_CLR *clr,		/* Base color of the Vertex */
	p3dc_PNT4 *light,	/* Optional light modifier (fractionates color) */
	p3dc_PNT2 *txt_coords)	/* Optional texture co-ordinates */
{
	float ooz = 1.0f / pq->tv.w;
	if (clr == NULL) clr = &COLOR_white; /* default color */
	g->oow = 1.0f / ((pq->tv.z * 65535.0f)+1.0f);

	/* While debugging, enforce clipping */
#ifdef CLIP_DEBUG
	assert(fabs(pq->tv.x * g->oow)<= 1.0001f);
	assert(fabs(pq->tv.y * g->oow)<= 1.0001f); 
#endif

	g->x = (float) (pq->tv.x * ooz * vp->scale_x + vp->center_x + SNAP_CONSTANT); 
	g->y = (float) (pq->tv.y * ooz * vp->scale_y + vp->center_y + SNAP_CONSTANT);

	if (txt_coords) {
		g->tmuvtx[0].sow = txt_coords->u * g->oow;
		g->tmuvtx[0].tow = txt_coords->v * g->oow;
	}

	if (light) {
		(g)->r = light->x * (float) (clr->gRED);
		(g)->g = light->y * (float) (clr->gGREEN);
		(g)->b = light->z * (float) (clr->gBLUE);
	} else {
		(g)->r = clr->gRED;
		(g)->g = clr->gGREEN;
		(g)->b = clr->gBLUE;
	}

	(g)->a = (float) (clr->gALPHA);
}


/*
 * Draw a point on the screen using the grDrawPoint primitive.
 * This is computationally pretty expensive, we should access the
 * frame buffer directly, but since I don't draw that many points
 * I've not taken the time to optimize this.
 */
void
__hw_draw_point(p3dc_VIEW *vp, p3dc_POINT *p) { 
	GrVertex p1;
	makeGlideVertex(&p1, vp, p->p0, p->color, &(p->light), NULL);
	__hw_disable_texture();
	grDrawPoint(&p1);
}

/*
 * Draw a line in the frame buffer. The vertices are assumed to be in
 * world co-ordinates and are transformed into view co-ordinates using
 * the P matrix in the camera.
 */
void
__hw_draw_line(p3dc_VIEW *vp, p3dc_LINE *ll) {
	GrVertex p1, p2;
	makeGlideVertex(&p1, vp, ll->p1, ll->color, &(ll->light), NULL);
	makeGlideVertex(&p2, vp, ll->p2, ll->color, &(ll->light), NULL);
	__hw_disable_texture();
	grDrawLine(&p1, &p2);
}

static GrVertex *vtxArray;
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

	while (1) {
		for (t = vL->head, n = 0; t != NULL; t = t->nxt, n++) {
			if (n == vtxSize) {
				if (vtxArray) {
					free(vtxArray);
				}
				vtxArray = (GrVertex *)malloc((vtxSize+10)*sizeof(GrVertex));
				vtxSize += 10;
				break;
			}
			makeGlideVertex(vtxArray+n, vp, (p3dc_VRTX *)(t->data.p), pp->color,
															   &(pp->light),
															   NULL);
		}
		__hw_disable_texture();
		if (t == NULL) {
			grDrawPolygonVertexList(n, vtxArray);
			return;
		}
	}
}

extern void __hw_disable_texture(void);
extern void __hw_enable_texture(void *);

/*
 * Draw a face potentially with gourard shading.
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
__hw_draw_face(p3dc_VIEW *vp, p3dc_FACE *ff, p3dc_LIST *(lists[]), void *texture, int lines) {
	p3dc_NODE *t_vertex = NULL;
	p3dc_NODE *t_light = NULL;
	p3dc_NODE *t_texture = NULL;
	p3dc_LIST *vertex_list = lists[0]; /* The vertices of the face */
	p3dc_LIST *light_list = lists[1]; /* Potentially an array of lights */
	p3dc_LIST *texture_list = lists[2]; /* Potentially a decal texture map */
	p3dc_POLY *pp = &(ff->poly);
#ifdef EARLY_CULL
	p3dc_VRTX *vv;
	p3dc_FLOAT oow, c;
	p3dc_PNT3 p1, p2, p3;
#endif
	int num_verts, i;

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
				vtxArray = (GrVertex *)malloc((vtxSize+10)*sizeof(GrVertex));
				assert(vtxArray != NULL);
				vtxSize += 10;
				break; /* Jump out of the inner loop and restart! */
			}
#ifdef EARLY_CULL
			/*
			 * Process the vertices to see if we can cull the polygon
			 * (and thus not draw it)
			 */
			if (num_verts < 3) {
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
						if (c > 0) 
							return;
				}
			}
#endif
			makeGlideVertex(vtxArray+num_verts, vp, 
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
		if (texture == NULL) {
			__hw_disable_texture();
		} else {
			__hw_enable_texture(texture);
		}
		switch (num_verts) {
			case 0:
				return;
			case 1:
				grDrawPoint(vtxArray);
				return;
			case 2:
				grDrawLine(vtxArray, vtxArray+1);
				return;
			default:
				if (lines) {
					for (i = 0; i < num_verts-1; i++) {
						grDrawLine(vtxArray+i, vtxArray+1+i);
					}
					grDrawLine(vtxArray+num_verts-1, vtxArray);
				} else
					grDrawPolygonVertexList(num_verts, vtxArray);
				return;
		}
	}
}

static GrColor_t fog_color;
static fogging = 0;

/*
 * Open up glide.
 */
int
__hw_init() {
	grGlideInit();
	QueryPerformanceFrequency(&ticks_per_count);

	if (! grSstQueryHardware(&__p3dc_glide_hw)) {
		return 1;
	}
	memset((char *)fog, 0, sizeof(fog));
	guFogGenerateLinear(fog, 2, 100);

	grSstSelect(0); /* Primary VooDoo */
	grSstWinOpen(0l, GR_RESOLUTION_640x480, GR_REFRESH_120Hz, GR_COLORFORMAT_RGBA,
					GR_ORIGIN_LOWER_LEFT, 2, 1);
	grDepthBufferMode(GR_DEPTHBUFFER_WBUFFER);
	grDepthBufferFunction(GR_CMP_LESS);
	grDepthMask(FXTRUE);
	grColorCombine( GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE,
						GR_COMBINE_LOCAL_ITERATED, GR_COMBINE_OTHER_NONE, FXFALSE);

	grCullMode(GR_CULL_NEGATIVE);
	grFogTable(fog);
	return 0;
}

static void
set_transparent(int a) {
	grConstantColorValue(a);
	grAlphaCombine(GR_COMBINE_FUNCTION_BLEND_OTHER, GR_COMBINE_FACTOR_ONE,
			GR_COMBINE_LOCAL_NONE, GR_COMBINE_OTHER_CONSTANT, FXFALSE);
	grAlphaBlendFunction(GR_BLEND_SRC_ALPHA, GR_BLEND_ONE_MINUS_SRC_ALPHA,
		GR_BLEND_ONE, GR_BLEND_ZERO);
}

static void
set_opaque() {
	grAlphaBlendFunction(GR_BLEND_ONE, GR_BLEND_ZERO, GR_BLEND_ONE, GR_BLEND_ZERO);
}

static void
set_lightmap() {
	grAlphaBlendFunction(GR_BLEND_DST_COLOR, GR_BLEND_ONE, GR_BLEND_ONE, GR_BLEND_ZERO);
}

void
__hw_effect_fog(p3dc_CLR *fog) {
	if (fog) {
		grFogMode(GR_FOG_WITH_TABLE);
		fog_color = fog->rgba;
		grFogColorValue(fog_color);
		fogging = 1;
	} else {
		grFogMode(GR_FOG_DISABLE);
		fogging = 0;
	}
}

/*
 * prepare frame for rendering.
 */
void
__hw_prepare_frame(p3dc_CLR *bg) {
	grBufferClear(fogging ? fog_color : bg->rgba, 0, GR_WDEPTHVALUE_FARTHEST);
}

void
__hw_clear_region(p3dc_CLR *cl, float x, float y, float w, float h) {
	GrVertex p1, p2, p3, p4;
	set_opaque();
	__hw_disable_texture();

	p1.x = x; p1.y = y; p2.x = x + w; p2.y = y;
	p3.x = x; p3.y = y+h; p4.x = x+w; p4.y = y+h;
	p1.oow = p2.oow = p3.oow = p4.oow = 0; // GR_WDEPTHVALUE_FARTHEST;
	p1.r = p2.r = p3.r = p4.r = cl->gRED;
	p1.g = p2.g = p3.g = p4.g = cl->gGREEN;
	p1.b = p2.b = p3.b = p4.b = cl->gBLUE;
	p1.a = p2.a = p3.a = p4.a = 255.0f;
	grDepthBufferFunction(GR_CMP_ALWAYS);
	grDrawTriangle(&p2, &p3, &p1);
	grDrawTriangle(&p2, &p4, &p3);
	grDepthBufferFunction(GR_CMP_LESS);
}
void
__hw_show_frame(int wait) {
	int pend, loopcnt, prepend;
	if (! wait) {
		grBufferSwap(0);
		return;
	}
	loopcnt = 0;
	prepend = 0;
	while ((pend = grBufferNumPending()) > 2) {
		if (prepend == pend)
			loopcnt++;
		else {
			prepend = pend;
			loopcnt = 0;
		}
		if (loopcnt > 4096) {
			p3dc_log("Loop count overflow!\n");
			loopcnt = 0;
		}
	}
	grBufferSwap(wait);
}

/*
 * Close glide.
 */
void
__hw_shutdown() {
	grGlideShutdown();
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
		*p2 = grSstScreenHeight();
		*p1 = grSstScreenWidth();
		res = 0;
		break;
	case P3DC_CTRL_DISPLAY:
		a = va_arg(vlst, int);
		switch (a) {
		case P3DC_DISPLAY_OFF:
			res = ! grSstControl(GR_CONTROL_DEACTIVATE);
			break;
		case P3DC_DISPLAY_ON:
			res = ! grSstControl(GR_CONTROL_ACTIVATE);
			break;
		case P3DC_DISPLAY_MOVE:
			res = ! grSstControl(GR_CONTROL_MOVE);
			break;
		case P3DC_DISPLAY_RESIZE:
			res = ! grSstControl(GR_CONTROL_RESIZE);
			break;
		default:
			res = 1;
		}
		break;
	case P3DC_CTRL_DEPTHBUFFER:
		a = va_arg(vlst, int);
		if (a) 
		    grDepthBufferFunction(GR_CMP_LESS);
		else
			grDepthBufferFunction(GR_CMP_ALWAYS);
		res = 0;
		break;
	case P3DC_CTRL_GSHADING:
		a = va_arg(vlst, int);
		__do_g_shading = (a != 0);
		break;
	case P3DC_CTRL_TRANSPARENT:
		a = va_arg(vlst, int);
		set_transparent(a);
		break;
	case P3DC_CTRL_LIGHTMAP:
		a = va_arg(vlst, int);
		if (a)
			set_lightmap();
		else
			set_opaque();
		break;
	case P3DC_CTRL_OPAQUE:
		set_opaque();
		break;
	case P3DC_CTRL_DEPTHBIAS:
		a = va_arg(vlst, int);
		grDepthBiasLevel((short) a);
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
 * This is the actual code for reading the contents of the linear frame
 * buffer from the Voodoo card.
 */
int
__hw_get_row(int row, unsigned char data[]) {
	GrLfbInfo_t	myLock;
	unsigned long *pixels;
	int i;

	memset((char *)&myLock, 0, sizeof(myLock));
	myLock.size = sizeof(myLock);

	/* try to get a lock on the linear frame buffer */
	if (! grLfbLock(GR_LFB_READ_ONLY, GR_BUFFER_FRONTBUFFER, GR_LFBWRITEMODE_565,
		GR_ORIGIN_LOWER_LEFT, 0, &myLock)) {
		p3dc_log("Could not lock the frame buffer.\n");
		return 1;
	}

	/* Note we reverse the rows because PNG things the top left is the
	 * first row.
	 */
	row = 479 - row;
	pixels = (unsigned long *)((unsigned char *)(myLock.lfbPtr) + row * myLock.strideInBytes);
	for (i = 0; i < 640; i += 2, pixels++) {
		unsigned short p0, p1;
		unsigned long px = *pixels;
		p0 = (unsigned short)(px & 0xffff);
		p1 = (unsigned short)((px >> 16) & 0xffff);
		data[i*3+0] = (unsigned char)((p0 >> 8) & 0xf8);
		data[i*3+1] = (unsigned char)((p0 >> 3) & 0xfc);
		data[i*3+2] = (unsigned char)((p0 << 3) & 0xf8);
		data[(i+1)*3+0] = (unsigned char)((p1 >> 8) & 0xf8);
		data[(i+1)*3+1] = (unsigned char)((p1 >> 3) & 0xfc);
		data[(i+1)*3+2] = (unsigned char)((p1 << 3) & 0xf8);
	}
	grLfbUnlock(GR_LFB_READ_ONLY, GR_BUFFER_FRONTBUFFER);
	return 0;
}
#endif