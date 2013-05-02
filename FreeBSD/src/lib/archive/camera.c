/*
 * camera.c - Implementation of the camera routines
 * $Id: camera.c,v 1.1 1999/08/18 01:35:19 cmcmanis Exp cmcmanis $
 *
 * This module implements cameras in the generic 3D library. A camera is
 * basically a viewport on the scene. It is positioned, pointed and
 * its field of view is set, then it can be used to render the scene
 * f  rom that view.
 *
 * Change History:
 *	$Log: camera.c,v $
 *	Revision 1.1  1999/08/18 01:35:19  cmcmanis
 *	Initial revision
 *
 *	Revision 1.1  1999/08/18 01:05:58  cmcmanis
 *	Initial revision
 *
 *	Revision 1.11  1999/08/16 09:51:26  cmcmanis
 *	Woo-hoo. Textures work again.
 *
 *	Revision 1.10  1999/07/26 03:10:31  cmcmanis
 *	Renaming is complete, ModelTest runs again, the view was added to support Models sharing the same draw functions as other programs.
 *
 *	Revision 1.9  1999/07/25 09:11:52  cmcmanis
 *	The great renaming from g3d/G3D -> p3dc/P3DC
 *
 *	Revision 1.8  1998/11/16 03:41:01  cmcmanis
 *	Updated clipping
 *
 *	Revision 1.8  1998/11/08 05:16:28  cmcmanis
 *	Outdated (renamed to 'H:\P3DC-RCS\src\lib\camera.c,v').
 *
 *	Revision 1.7  1998/10/18 09:29:49  cmcmanis
 *	Models now position and move correctly. I've re-ordered my matrices
 *	to be SPHIGs compatible so that I can use SPHIGS to test my results.
 *	Created p3dc_camera_xfrm and p3dc_orientation_xfrm as compliments of
 *	each other. Cool stuff there.
 *
 *	Revision 1.6  1998/10/10 07:00:27  cmcmanis
 *	Model parsing is written (not fully debugged) and the texture structures
 *	are closer to their final form. By the next check in we should have a
 *	few simple models up and running with textured models to follow.
 *
 *	Revision 1.5  1998/09/23 08:48:50  cmcmanis
 *	Clipping works! Polygon and line clipping is pretty robust at this point
 *	still looking for corner cases but I believe all of the code is correct at
 *	this point.
 *
 *	Revision 1.4  1998/09/20 08:10:44  cmcmanis
 *	Major rewrite, added list stuff, and primitives. Almost completely
 *	back to where version 4 was at this point. This version is ALPHA0_2
 *
 *	Revision 1.3  1998/09/18 08:34:43  cmcmanis
 *	This update has new clipping code for polygons.
 *	In particular the nodes are not needlessly copied.
 *
 *	Adds the new list capability in util.c/memory.c that
 *	uses a p3dc_NLST (type P3DC_LIST) as a simple
 *	linked list.
 *
 *	Revision 1.2  1998/09/03 19:52:12  cmcmanis
 *	Cube Test runs, camera and transformation code checks out, 
 *  lighting still does not work correctly.
 *
 *	Revision 1.1  1998/09/02 08:48:25  cmcmanis
 *	Initial revision
 *
 *	Revision 1.1  1998/08/30 09:46:52  cmcmanis
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
#include "p3dc.h"

extern void p3dc_camera_xfrm(p3dc_XFRM *, p3dc_PNT3 *, p3dc_PNT3 *, p3dc_FLOAT);

/*
 * This function computes the perspective transform and then
 * adds that to the existing viewing transform to get the final
 * convert a point from the world into nearly ready-to-render
 * transform.
 * 
 * The perspective transform is basically this:
 * | 1/xaspect  0.0       0.0            0.0     |
 * | 0.0        1.0       0.0            0.0     |
 * | 0.0        0.0  s/(d * (1-d/f))  -s/(1-d/f) |
 * | 0.0        0.0       s/d            0.0     |
 *
 *                           zs             s
 * Which transforms z  = ----------  -  ---------
 *                        d(1-d/f)       (1-d/f) 
 *
 *                           zs
 * And w simply becomes w = ----
 *                            d
 *
 * Now when w == 1.0, z == d (ie on the viewplane) when z > Zfar, z > w.
 *
 * The other thing this does is scale the X co-ordinate by 1/xaspect. The
 * effect of this is that the X co-ordinate can be clipped to -w < x < w 
 * and then re-expanded to undo this change. This saves a bunch of multiplies
 * in the clip code.
 */
static void
__p3dc_compute_P(p3dc_CMRA *cam, int use_d) {
	p3dc_FLOAT d;
	p3dc_FLOAT fov = cam->fov;
	p3dc_XFRM t;
	p3dc_VIEW *view = &(cam->vp);

	memcpy((char *)(&(cam->view_xfrm)), (char *)&(cam->world_xfrm), sizeof(p3dc_XFRM));
	p3dc_identity_xfrm(&t);
	fov = (p3dc_FLOAT)((fov * PI)/360.0);
	if (! use_d)
		cam->d = d = (p3dc_FLOAT) 1.0 / (p3dc_FLOAT) tan(fov);
	else
		d = cam->d;

	t.data[0][0] = view->scale_y/view->scale_x;
	t.data[3][3] = 0;
	t.data[3][2] = (p3dc_FLOAT)  1.0 / d;
	t.data[2][2] = (p3dc_FLOAT)  1.0 / (d * ((p3dc_FLOAT) 1.0 - d/cam->zclip));
	t.data[2][3] = (p3dc_FLOAT) -1.0 / ((p3dc_FLOAT) 1.0 - d/cam->zclip);
	p3dc_mult_xfrm(&(cam->view_xfrm), &t, &(cam->view_xfrm));
}

/*
 * Computes a viewing transform V that will convert world co-ordinates 
 * into 'eye' co-ordinates. This function assumes that both the eye
 * position (eye), and the direction in which you are looking (look)
 * are both defined.
 *
 * NOTE: This algorithm assumes a LEFT HAND co-ordinate system (+z goes
 *       into the distance.)
 *
 * This was the old algorithm, and it worked so I've retained it
 * in this comment but the current algorithm is much simpler.
 *
 *	1) Compute a vector for the direction the "eyeball" is looking, 
 *     this is simply:
 *
 *			Veye - Vlook                                  (1-1)
 *
 *	2) Now translate the world, such that the eyeball
 *	   location is the origin. This is:
 *
 *          T(-eye.x, -eye.y, -eye.z)                     (2-1)
 *
 *	3) Now compute a rotation of the world about the Y
 *     axis, such that the view vector will lie completely
 *     in the YZ plane. (ie its X value is 0).
 *
 *     This computation is done by projecting our view vector
 *     into the ZX plane, and determining the angle between
 *     the vector and the Z axis. Given our projected vector,
 *     the following equation is true:
 *
 *         Lz = cos(theta) * Lzx                          (3-1)
 *
 *     Where Lz is the vector's length on the Z axis and
 *     Lzx is its length in the ZX plane. Solving for theta
 *     gives us:
 *
 *            -1
 *         cos  (Lz/Lzx) = theta                          (3-2)
 *
 *
 *     However, the answer will be between 0 and 180 rather than
 *	   between 0 and 360. (the cos function "wraps around") so we
 *	   compare the X component, if it is positive, then the angle
 *	   described is "negative", otherwise we use it as it.
 *
 *  4) Now we transform our view vector with this rotation. This
 *     gives us a new View direction that has only Y and Z components.
 *     We use this to compute the rotation about the X axis that will
 *     cause the vector to lie completely on the Z axis. (ie its Y
 *     value is 0).
 *
 *     This computation is done by noting that the angle is again
 *     a function of the length of the vector and its length on the
 *     Y axis as follows:
 *
 *         Lz' = cos(theta) * Lzy'                         (4-1)
 *
 *     Where Ly' is the transformed length on the Y axis and the
 *     value Lzy' is the length in the ZY plane of the transformed
 *     view vector. Again, solving for theta gives us:
 *
 *            -1
 *         cos  (Lz'/Lzy') = theta                         (4-2)
 *
 *	   In this case if Y is negative we know the angle is negative. 
 *	   The previous transformation forced the system to have
 *	   a positive Z co-ordinate.
 *
 *     Given the value for theta, we rotate the world about the X axis
 *     this many degrees.
 *
 *  We're done! Now any point in world co-ordinates that we transform
 *  with this matrix will be in 'view' (or eye) co-ordinates. A good
 *  test case for this code is to transform a point at (1, 1, 1) when
 *  your eyeball is at (10, 10, -10) and you are looking at (0, 0, 0).
 *  The resulting point should be directly in front of the eyeball on
 *  the Z axis. If not, then you've got a bug somewhere!
 *
 * The new algorithm works as follows:
 *	1.) Compute the Z axis as the unit vector pointing
 *		from the origin to the "look" point.
 *	2.) Compute the new X axis by finding the cross product
 *		of that Z vector and the Y axis, if the Z vector
 *		IS the Y axis then use the old -Z axis as the new
 *		Y.
 *	3.) Compute an orthogonal Y axis by computing the cross
 *		product of X and Z.
 *	4.) Multiply this by the location of the origin (translate
 *		to the camera's origin)
 *	5.) Then finally compute the new perspective transform.
 *
 */
static void
__p3dc_compute_V(p3dc_CMRA *cam) {
	p3dc_PNT3 nz; 
	p3dc_XFRM *v;

	v = &(cam->world_xfrm);

	/* 
	 * Start by computing our "z" vector. This is defined to
	 * be the difference between where we are (origin) and
	 * the point at which we are looking. (look)
	 */

	nz.x = cam->look.x - cam->vp.origin.x;
	nz.y = cam->look.y - cam->vp.origin.y;
	nz.z = cam->look.z - cam->vp.origin.z;

	p3dc_camera_xfrm(v, &nz, &(cam->vp.origin), cam->roll);
	/*
	 * Finally, compute the perspective projection matrix and
	 * and multiply by the view matrix to get the final transform.
	 */ 
	__p3dc_compute_P(cam, 0);
}

/* We need to update this */
void
p3dc_init_camera(p3dc_CMRA *cam, p3dc_FLOAT x, p3dc_FLOAT y, p3dc_FLOAT z) {
	cam->vp.origin.x = x;
	cam->vp.origin.y = y;
	cam->vp.origin.z = z;
	cam->fov = (p3dc_FLOAT) 35.03;
	cam->d = 0.0;
	cam->look.x = cam->look.y = cam->look.z = 0;
	cam->roll = 0;
	cam->vp.scale_y = 240;
	cam->vp.scale_x = 320;
	cam->vp.center_x = 320;
	cam->vp.center_y = 240;
	cam->zclip = 2048.0;
	p3dc_identity_xfrm(&(cam->world_xfrm));
	__p3dc_compute_V(cam);
}

/*
 * Figure out how zoom factors work and re-name this...
 */
void
p3dc_fov_camera(p3dc_CMRA *cam, p3dc_FLOAT fov) {
	cam->fov = fov;
	__p3dc_compute_P(cam, 0);
}

p3dc_CMRA *
p3dc_new_camera(p3dc_FLOAT fov, p3dc_FLOAT x, p3dc_FLOAT y, p3dc_FLOAT z) {
	p3dc_CMRA *res;

	res = (p3dc_CMRA *)calloc(1, sizeof(p3dc_CMRA));
	p3dc_init_camera(res, x, y, z);
	p3dc_fov_camera(res, fov);
	return res;
}


void
p3dc_locate_camera(p3dc_CMRA *cam, p3dc_FLOAT x, p3dc_FLOAT y, p3dc_FLOAT z) {
	cam->vp.origin.x = x;
	cam->vp.origin.y = y;
	cam->vp.origin.z = z;
	__p3dc_compute_V(cam);
}

void
p3dc_target_camera(p3dc_CMRA *cam, p3dc_FLOAT x, p3dc_FLOAT y, p3dc_FLOAT z) {
	cam->look.x = x;
	cam->look.y = y;
	cam->look.z = z;
	__p3dc_compute_V(cam);
}

void
p3dc_point_camera(p3dc_CMRA *cam, p3dc_FLOAT x, p3dc_FLOAT y, p3dc_FLOAT z) {
	p3dc_PNT3 norm;
	p3dc_FLOAT len;

	norm.x = x; norm.y = y; norm.z = z;
	len = 1.0f / p3dc_dot_pnt3(&norm, &norm);
	cam->look.x = cam->vp.origin.x + (x * len);
	cam->look.y = cam->vp.origin.y + (y * len);
	cam->look.z = cam->vp.origin.z + (z * len);
	__p3dc_compute_V(cam);
}

void
p3dc_roll_camera(p3dc_CMRA *cam, p3dc_FLOAT ang) {
	cam->roll = ang;
	__p3dc_compute_V(cam);
}

void
p3dc_window_camera(p3dc_CMRA *cam, int x, int y) {
	cam->vp.scale_y = (float) (y/2.0f);
	cam->vp.scale_x = cam->vp.scale_y *  (float) (x/y);
	cam->vp.center_x = (p3dc_FLOAT)((p3dc_FLOAT)x/2.0f);
	cam->vp.center_y = (p3dc_FLOAT)((p3dc_FLOAT)y/2.0f);
}

void
p3dc_frame_camera(p3dc_CMRA *cam, int lx, int ly, int ux, int uy) {
	int x, y;

	x = ux - lx;
	y = uy - ly;

	cam->vp.scale_y = (float) y / 2.0f;
	cam->vp.scale_x = (float)(x/ 2.0);
	cam->vp.center_x = (float) (lx + x/2);
	cam->vp.center_y = (float) (ly + y/2);
}

void
p3dc_zclip_camera(p3dc_CMRA *cam, p3dc_FLOAT zfar) {
	cam->zclip = zfar;
	__p3dc_compute_P(cam, 0);
}

void
p3dc_setd_camera(p3dc_CMRA *cam, p3dc_FLOAT newD) {
	cam->d = newD;
	__p3dc_compute_P(cam, 1);
}
