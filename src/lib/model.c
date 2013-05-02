/*
 * model.c -- simple model structure
 * $Id: model.c,v 1.5 1999-10-31 15:18:10-08 cmcmanis Exp cmcmanis $
 * 
 * This module defines a simple module structure and its
 * uses.
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
 * Change history:
 *	$Log: model.c,v $
 *	Revision 1.5  1999-10-31 15:18:10-08  cmcmanis
 *	Additional camera fixes, now we have one attribute setting function with
 *	variable arguments.
 *
 *	Revision 1.4  1999/09/12 06:31:11  cmcmanis
 *	Rewrote clipping api (again?)
 *
 *	Revision 1.2  1999/09/02 03:22:40  cmcmanis
 *	The great LIST rewrite, lists are much cleaner now
 *
 *	Revision 1.1  1999/08/18 01:33:50  cmcmanis
 *	Initial revision
 *
 *	Revision 1.1  1999/08/18 01:06:17  cmcmanis
 *	Initial revision
 *
 *	Revision 1.11  1999/08/16 09:51:21  cmcmanis
 *	Woo-hoo. Textures work again.
 *
 *	Revision 1.10  1999/08/14 23:18:38  cmcmanis
 *	This version now includes a new model structure that allows
 *	for dynamic changing of colors and textures in the model
 *	after it has been loaded.
 *
 *	Revision 1.9  1999/07/26 03:10:27  cmcmanis
 *	Renaming is complete, ModelTest runs again, the view was added to support Models sharing the same draw functions as other programs.
 *
 *	Revision 1.8  1999/07/25 09:11:54  cmcmanis
 *	The great renaming from g3d/G3D -> p3dc/P3DC
 *
 *	Revision 1.7  1998/11/16 03:40:58  cmcmanis
 *	Updated clipping
 *
 *	Revision 1.7  1998/11/08 05:20:49  cmcmanis
 *	Outdated (renamed to 'H:\P3DC-RCS\src\lib\model.c,v').
 *
 *	Revision 1.6  1998/11/05 20:23:42  cmcmanis
 *	Checkin prior to moving into a better source tree structure
 *
 *	Revision 1.5  1998/10/18 09:29:44  cmcmanis
 *	Models now position and move correctly. I've re-ordered my matrices
 *	to be SPHIGs compatible so that I can use SPHIGS to test my results.
 *	Created p3dc_camera_xfrm and p3dc_orientation_xfrm as compliments of
 *	each other. Cool stuff there.
 *
 *	Revision 1.4  1998/10/13 21:04:14  cmcmanis
 *	Model parsing and basic drawing is checked out. The model positioning
 *	code and relighting code still needs work.
 *
 *	Revision 1.3  1998/10/11 06:42:33  cmcmanis
 *	Model parsing works as does model dumping, free_face
 *	still needs to be written.
 *
 *	Revision 1.2  1998/10/10 07:00:41  cmcmanis
 *	Model parsing is written (not fully debugged) and the texture structures
 *	are closer to their final form. By the next check in we should have a
 *	few simple models up and running with textured models to follow.
 *
 *	Revision 1.1  1998/10/08 08:33:55  cmcmanis
 *	Initial revision
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifdef __MSC__
#include <fcntl.h>
#include <io.h>
#else
#include <sys/fcntl.h>
#define _O_RDONLY O_RDONLY
#define _open open
#define _read read
#define _close close
#endif
#include <assert.h>
#include <math.h>
#include <p3dc.h>

/* this function keeps track of the next available transform ID */
extern int __p3dc_get_xform_id(void);

void
p3dc_locate_model(p3dc_MODEL *m, p3dc_FLOAT x, p3dc_FLOAT y, p3dc_FLOAT z) {
	m->loc.x = x; m->loc.y = y; m->loc.z = z;
	m->O.data[0][3] = x;
	m->O.data[1][3] = y;
	m->O.data[2][3] = z;
	m->O.id = __p3dc_get_xform_id();
	m->flags = 0;
}

void
p3dc_target_model(p3dc_MODEL *m, p3dc_FLOAT x, p3dc_FLOAT y, p3dc_FLOAT z) {
	p3dc_PNT3 dir;
	float len;

	dir.x = x - m->loc.x;
	dir.y = y - m->loc.y;
	dir.z = z - m->loc.z;
	len = p3dc_dot_pnt3(&dir, &dir);
	if (len == 0)
		return;
	p3dc_orientation_xfrm(&(m->O), &dir, &(m->loc), NULL, 0);
	m->flags = 0;
}

void
p3dc_point_model(p3dc_MODEL *m, p3dc_PNT3 *newZ, p3dc_PNT3 *up, p3dc_FLOAT twist) {
	p3dc_PNT3 dir;
	float len;

	dir.x = newZ->x; 
	dir.y = newZ->y;
	dir.z = newZ->z;
	/* normalize Z */
	len = p3dc_dot_pnt3(&dir, &dir);
	assert(len > 0);
	p3dc_orientation_xfrm(&(m->O), &dir, &(m->loc), up, twist);
	m->flags = 0;
}

/*
 * This function resets the models orientation based on its
 * internal euler angles
 *
 * Note that it may be wrong given the manipulations of
 * the user...
 */
void
p3dc_reset_model(p3dc_MODEL *m) {
	p3dc_PNT3 ea = m->eulera;
	p3dc_identity_xfrm(&(m->O));
	p3dc_pitch_model(m, ea.x);
	p3dc_yaw_model(m, ea.y);
	p3dc_roll_model(m, ea.z);
	m->eulera = ea;
}

/*
 * roll the model about it's current Z axis. 
 * This is done in the following way.
 * Since we know the parametric equation for the X and Y axis
 * using the unit vectors in the orientation array, we compute
 * the new X, Y co-ordinate by adding the x,y,z point computed
 * along the line on X to the x,y,z point of the line computed along
 * Y. Vector arithmetic assures us this will be the point in the
 * XY plane. 
 *
 * The line from the origin to this point becomes our new Y axis
 * and since Z hasn't changed we cross it with Z to get the new
 * X axis. Now we're back to being orthonormal again!
 *
 * The down side of this technique is that it is always a
 * 'delta' from the current orientation so the internal Euler angle
 * will drift over time.
 */
void
p3dc_roll_model(p3dc_MODEL *m, p3dc_FLOAT ang) {
	p3dc_PNT3 xa, ya, za;
	float tx, ty;
	m->eulera.z += ang;

	tx = (p3dc_FLOAT)sin(ang * DTR);
	ty = (p3dc_FLOAT)cos(ang * DTR);
	ya.x = tx * m->O.data[0][0];
	ya.y = tx * m->O.data[1][0];
	ya.z = tx * m->O.data[2][0];
	ya.x += ty * m->O.data[0][1];
	ya.y += ty * m->O.data[1][1];
	ya.z += ty * m->O.data[2][1];
	p3dc_normalize_pnt3(&ya); /* fixes up round off error */
	za.x = m->O.data[0][2];
	za.y = m->O.data[1][2];
	za.z = m->O.data[2][2];
	p3dc_cross_pnt3(&ya, &za, &xa);
	p3dc_normalize_pnt3(&xa);
	m->O.data[0][0] = xa.x; m->O.data[1][0] = xa.y; m->O.data[2][0] = xa.z;
	m->O.data[0][1] = ya.x; m->O.data[1][1] = ya.y; m->O.data[2][1] = ya.z;
	m->O.id = __p3dc_get_xform_id();
	m->flags = 0;
}

/*
 * Pitching the model changes the Z axis and leaves the X axis intact.
 */
void
p3dc_pitch_model(p3dc_MODEL *m, p3dc_FLOAT ang) {
	p3dc_PNT3 xa, ya, za;
	float tx, ty;
	m->eulera.x += ang;

	tx = (p3dc_FLOAT)sin(ang * DTR);
	ty = (p3dc_FLOAT)cos(ang * DTR);
	za.x = ty * m->O.data[0][2];
	za.y = ty * m->O.data[1][2];
	za.z = ty * m->O.data[2][2];
	za.x += tx * m->O.data[0][1];
	za.y += tx * m->O.data[1][1];
	za.z += tx * m->O.data[2][1];
	p3dc_normalize_pnt3(&za); /* fixes up round off error */
	xa.x = m->O.data[0][0];
	xa.y = m->O.data[1][0];
	xa.z = m->O.data[2][0];
	p3dc_cross_pnt3(&za, &xa, &ya);
	p3dc_normalize_pnt3(&ya);
	m->O.data[0][2] = za.x; m->O.data[1][2] = za.y; m->O.data[2][2] = za.z;
	m->O.data[0][1] = ya.x; m->O.data[1][1] = ya.y; m->O.data[2][1] = ya.z;
	m->O.id = __p3dc_get_xform_id();
	m->flags = 0;
}

/*
 * Yawing the model changes the Z axis and leaves the Y axis intact.
 */
void
p3dc_yaw_model(p3dc_MODEL *m, p3dc_FLOAT ang) {
	p3dc_PNT3 xa, ya, za;
	float tx, ty;
	m->eulera.y += ang;

	tx = (p3dc_FLOAT)sin(ang * DTR);
	ty = (p3dc_FLOAT)cos(ang * DTR);
	za.x = ty * m->O.data[0][2];
	za.y = ty * m->O.data[1][2];
	za.z = ty * m->O.data[2][2];
	za.x += tx * m->O.data[0][0];
	za.y += tx * m->O.data[1][0];
	za.z += tx * m->O.data[2][0];
	p3dc_normalize_pnt3(&za); /* fixes up round off error */
	ya.x = m->O.data[0][1];
	ya.y = m->O.data[1][1];
	ya.z = m->O.data[2][1];
	p3dc_cross_pnt3(&ya, &za, &xa);
	p3dc_normalize_pnt3(&xa);
	m->O.data[0][2] = za.x; m->O.data[1][2] = za.y; m->O.data[2][2] = za.z;
	m->O.data[0][0] = xa.x; m->O.data[1][0] = xa.y; m->O.data[2][0] = xa.z;
	m->O.id = __p3dc_get_xform_id();
	m->flags = 0;
}

/*
 * Move the model "forward" along its current
 * Z axis by units length.
 */
void
p3dc_fwd_model(p3dc_MODEL *m, p3dc_FLOAT units) {
	p3dc_PNT3 dir;

	dir.x = m->O.data[0][2] * units;
	dir.y = m->O.data[1][2] * units;
	dir.z = m->O.data[2][2] * units;
	m->O.data[0][3] += dir.x;
	m->O.data[1][3] += dir.y;
	m->O.data[2][3] += dir.z;
	m->O.id = __p3dc_get_xform_id();
	m->flags = 0;
}

/*
 * Move the model "backward" along its current Z axis by
 * units length.
 */
void
p3dc_bkwd_model(p3dc_MODEL *m, p3dc_FLOAT units) {
	p3dc_PNT3 dir;

	dir.x = m->O.data[0][2] * units;
	dir.y = m->O.data[1][2] * units;
	dir.z = m->O.data[2][2] * units;
	m->O.data[0][3] -= dir.x;
	m->O.data[1][3] -= dir.y;
	m->O.data[2][3] -= dir.z;
	m->loc.x = m->O.data[0][3];
	m->loc.y = m->O.data[1][3];
	m->loc.z = m->O.data[2][3];
	m->O.id = __p3dc_get_xform_id();
	m->flags = 0;
}

/*
 * Change the position of the model relative to its current
 * position.
 */
void
p3dc_move_model(p3dc_MODEL *m, p3dc_FLOAT x, p3dc_FLOAT y, p3dc_FLOAT z) {
	m->O.data[0][3] = m->loc.x = (m->loc.x + x);
	m->O.data[1][3] = m->loc.y = (m->loc.y + y);
	m->O.data[2][3] = m->loc.z = (m->loc.z + z);
	m->O.id = __p3dc_get_xform_id();
	m->flags = 0;
}

/*
 * This function sets the named color in the model to have the value
 * passed as a color.
 * 
 * Returns -1 if the name wasn't found.
 *			-2 if the name wasn't a color.
 *			-3 if the name was ill formed.
 *		0 on success.
 */
int
p3dc_model_set_color(p3dc_MODEL *m, char *name, p3dc_CLR *clr) {
	p3dc_NODE *n;
	p3dc_CLR *cc;

	if ((name == NULL) || (strlen(name) > 63))
		return -3;

	n = p3dc_get_node(&(m->parts), P3DC_NODE_BYNAME, name);
	if (n == NULL)
		return -1;
	if (n->data.t != P3DC_CLR)
		return -2;
	cc = (p3dc_CLR *)(n->data.p);
	*cc = *clr; /* copy the name contents */
	return 0;
}

/*
 * This function sets the texture in the model to have the value
 * passed as a texture.
 * 
 * Returns -1 if the name wasn't found.
 *			-2 if the name wasn't a color.
 *			-3 if the name was ill formed.
 *			-4 texture could not be loaded.
 *		0 on success.
 */
int
p3dc_model_set_texture(p3dc_MODEL *m, char *name, char *texture) {
	p3dc_NODE *n;
	p3dc_TDATA *cc;

	if ((name == NULL) || (strlen(name) > 63))
		return -3;

	n = p3dc_get_node(&(m->parts), P3DC_NODE_BYNAME, name);
	if (n == NULL)
		return -1;
	if (n->data.t != P3DC_TDATA)
		return -2;
	cc = (p3dc_TDATA *)(n->data.p);
	if (p3dc_load_texture(texture, cc))
		return -4;
	return 0;
}

/*
 * This function sets *ALL* of the textures in the model to have the value
 * passed as a texture.
 * 
 * Returns -1 if the name wasn't found.
 *			-2 if the name wasn't a color.
 *			-3 if the name was ill formed.
 *			-4 texture could not be loaded.
 *		0 on success.
 */
int
p3dc_model_set_all_textures(p3dc_MODEL *m, char *texture) {
	p3dc_NODE *n;
	p3dc_TDATA *cc;

	for (n = FIRST_NODE(&(m->parts)); n != NULL; n = NEXT_NODE(&(m->parts))) {
		if (n->data.t == P3DC_TDATA) {
			cc = (p3dc_TDATA *)(n->data.p);
			if (p3dc_load_texture(texture, cc))
				return -1;
		}
	}
	return 0;
}

/*
 * This function sets *ALL* of the colors in the model to have the 
 * passed in color value.
 * 
 * Returns -1 if the name wasn't found.
 *			-2 if the name wasn't a color.
 *			-3 if the name was ill formed.
 *			-4 texture could not be loaded.
 *		0 on success.
 */
int
p3dc_model_set_all_colors(p3dc_MODEL *m, p3dc_CLR *color) {
	p3dc_NODE *n;
	p3dc_CLR *cc;

	for (n = FIRST_NODE(&(m->parts)); n != NULL; n = NEXT_NODE(&(m->parts))) {
		if (n->data.t == P3DC_CLR) {
			cc = (p3dc_CLR *)(n->data.p);
			*cc = *color;
		}
	}
	return 0;
}
/*
 * This function sets the texture in the model to have the value
 * passed as a texture.
 * 
 * Returns -1 if the name wasn't found.
 *			-2 if the name wasn't a color.
 *			-3 if the name was ill formed.
 *		0 on success.
 */
int
p3dc_model_setvertex(p3dc_MODEL *m, char *name, p3dc_FLOAT x, p3dc_FLOAT y, p3dc_FLOAT z) {
	p3dc_NODE *n;
	p3dc_VRTX *cc;

	if ((name == NULL) || (strlen(name) > 63))
		return -3;

	n = p3dc_get_node(&(m->parts), P3DC_NODE_BYNAME, name);
	if (n == NULL)
		return -1;
	if (n->data.t != P3DC_VRTX)
		return -2;
	cc = (p3dc_VRTX *)(n->data.p);
	cc->v.x = x; cc->v.y = y; cc->v.z = z;
	cc->xid = 0;
	return 0;
}
