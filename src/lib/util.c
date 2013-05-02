/*
 * util.c - Various utility routines for the library
 * $Id: util.c,v 1.4 1999-09-11 23:30:48-07 cmcmanis Exp $
 *
 * This module implements the functions that don't fit any
 * where else. 
 * 
 * Change Log:
 *	$Log: util.c,v $
 *	Revision 1.4  1999/09/12 06:30:48  cmcmanis
 *	Rewrote clipping api (again?)
 *
 *	Revision 1.2  1999/09/02 03:22:50  cmcmanis
 *	The great LIST rewrite, lists are much cleaner now
 *
 *	Revision 1.1  1999/08/18 01:34:02  cmcmanis
 *	Initial revision
 *
 *	Revision 1.1  1999/08/18 01:06:24  cmcmanis
 *	Initial revision
 *
 *	Revision 1.9  1999/07/25 09:11:57  cmcmanis
 *	The great renaming from g3d/G3D -> p3dc/P3DC
 *
 *	Revision 1.8  1998/11/16 03:41:05  cmcmanis
 *	Updated clipping
 *
 *	Revision 1.8  1998/11/08 05:22:24  cmcmanis
 *	Outdated (renamed to 'H:\P3DC-RCS\src\lib\util.c,v').
 *
 *	Revision 1.7  1998/10/10 07:00:46  cmcmanis
 *	Model parsing is written (not fully debugged) and the texture structures
 *	are closer to their final form. By the next check in we should have a
 *	few simple models up and running with textured models to follow.
 *
 *	Revision 1.6  1998/09/23 08:49:01  cmcmanis
 *	Clipping works! Polygon and line clipping is pretty robust at this point
 *	still looking for corner cases but I believe all of the code is correct at
 *	this point.
 *
 *	Revision 1.5  1998/09/20 08:11:00  cmcmanis
 *	Major rewrite, added list stuff, and primitives. Almost completely
 *	back to where version 4 was at this point. This version is ALPHA0_2
 *
 *	Revision 1.4  1998/09/18 08:34:31  cmcmanis
 *	This update has new clipping code for polygons.
 *	In particular the nodes are not needlessly copied.
 *
 *	Adds the new list capability in util.c/memory.c that
 *	uses a p3dc_NLST (type P3DC_LIST) as a simple
 *	linked list.
 *
 *	Revision 1.3  1998/09/04 05:47:56  cmcmanis
 *	Lighting works for lines and points.
 *	New memory allocator is in place but not
 *	heavily used yet.
 *
 *	Revision 1.2  1998/09/03 19:52:14  cmcmanis
 *	Cube Test runs, camera and transformation code checks out, 
 *  lighting still does not work correctly.
 *
 *	Revision 1.1  1998/09/02 08:48:28  cmcmanis
 *	Initial revision
 *
 *	Revision 1.1  1998/08/30 09:46:46  cmcmanis
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
#include <math.h>
#include <assert.h>
#include "p3dc.h"
/* 
 * Implementation of the "standard" colors 
 */
#ifdef LITTLE_ENDIAN
p3dc_CLR COLOR_black		= {0x000000ff};
p3dc_CLR COLOR_red		= {0xff0000ff};
p3dc_CLR COLOR_green		= {0x00ff00ff};
p3dc_CLR COLOR_yellow	= {0xffff00ff};
p3dc_CLR COLOR_blue		= {0x0000ffff};
p3dc_CLR COLOR_purple	= {0xff00ffff};
p3dc_CLR COLOR_cyan		= {0x00ffffff};
p3dc_CLR COLOR_white		= {0xffffffff};
p3dc_CLR COLOR_lt_gray	= {0xc0c0c0ff};
p3dc_CLR COLOR_md_gray	= {0x808080ff};
p3dc_CLR COLOR_dk_gray	= {0x404040ff};
p3dc_CLR COLOR_brown		= {0x804000ff};
p3dc_CLR COLOR_orange	= {0xff7f00ff};
#else
#endif


/*
 * Open up the rendering back end. This has to do all the
 * initial setup.
 */
int
p3dc_open() {
	p3dc_light_init();
	return __hw_init();
}

/*
 * Close the rendering back end. This should clean up as
 * much as possible. The user program will exit at this
 * point.
 */
void
p3dc_close() {
	p3dc_log_close();
	__hw_shutdown();
}

/*
 * When drawing the loop is, 
 *		prepare_frame()/...render.../flip_frame()
 */
void
p3dc_prepare_frame(p3dc_CLR *c) {
	if (c == NULL) c = &COLOR_black;
	__hw_prepare_frame(c);
}

/* 
 * Clear region will set a region of the visible screen to a particular
 * color and with zero depth information. 
 */
void
p3dc_clear_region(p3dc_CLR *c, p3dc_FLOAT x, p3dc_FLOAT y, p3dc_FLOAT w, p3dc_FLOAT h) {
	__hw_clear_region(c, (float) x, (float) y, (float) w, (float) h);
}

void
p3dc_clear_viewport(p3dc_CLR *c, p3dc_VIEW *vp) {
	 p3dc_FLOAT x, y, w, h;
	 x = vp->center_x - vp->scale_x;
	 y = vp->center_y - vp->scale_y;
	 w = vp->scale_x * 2;
	 h = vp->scale_y * 2;
	__hw_clear_region(c, (float) x, (float) y, (float) w, (float) h);
}

/*
 * This function displays the frame that has been drawn.
 */
void
p3dc_flip_frame(int wait) {
	__hw_show_frame(wait);
}

/*
 * Color manipulator. Adjust color value by a scalar, alpha is
 * unchanged.
 */
void
p3dc_adjust_color(p3dc_CLR *src, p3dc_FLOAT val, p3dc_CLR *dst) {
	dst->c[p3dc_CLR_RED] = (unsigned char) (src->c[p3dc_CLR_RED] * val);
	dst->c[p3dc_CLR_BLUE] = (unsigned char) (src->c[p3dc_CLR_BLUE] * val);
	dst->c[p3dc_CLR_GREEN] = (unsigned char) (src->c[p3dc_CLR_GREEN] * val);
	dst->c[p3dc_CLR_ALPHA] = src->c[p3dc_CLR_ALPHA]; 
}

/*
 * Alpha manipulator  - Adjust alpha by a scalar, color is unchanged.
 */
void
p3dc_adjust_alpha(p3dc_CLR *src, p3dc_FLOAT val, p3dc_CLR *dst) {
	*dst = *src;
	dst->c[p3dc_CLR_ALPHA] = (unsigned char)(src->c[p3dc_CLR_ALPHA] * val);
}

/*
 * Apply a light to a color, this lets you change both color and alpha
 * in a single operation.
 */
void
p3dc_apply_light(p3dc_CLR *src, p3dc_PNT4 *val, p3dc_CLR *dst) {
	dst->c[p3dc_CLR_RED] = (unsigned char) (src->c[p3dc_CLR_RED] * val->x);
	dst->c[p3dc_CLR_BLUE] = (unsigned char) (src->c[p3dc_CLR_BLUE] * val->y);
	dst->c[p3dc_CLR_GREEN] = (unsigned char) (src->c[p3dc_CLR_GREEN] * val->z);
	dst->c[p3dc_CLR_ALPHA] = (unsigned char) (src->c[p3dc_CLR_ALPHA] * val->w);
}

/*
 * Normalize a PNT3 vector, return 1 if it has zero length
 */
int
p3dc_normalize_pnt3(p3dc_PNT3 *p) {
	p3dc_FLOAT len, ool;

	len = (p3dc_FLOAT)sqrt(p->x*p->x + p->y*p->y + p->z*p->z);
	if (EQZ(len))
		return 1;
	ool = 1.0f/len;
	p->x *= ool;
	p->y *= ool;
	p->z *= ool;
	return 0;
}

p3dc_TYPE
p3dc_get_type(void *q) {
	p3dc_NODE *n;
	if (q == NULL)
		return P3DC_UNKNOWN;

	n = (void *)(((char *)q) - sizeof(p3dc_NODE));
	/* Simple test to see if it is a valid type ID */
	if (n->data.p == q)
		return n->data.t;

	/* If not return type unknown */
	return P3DC_UNKNOWN;
}
