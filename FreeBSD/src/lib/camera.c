/*
 * camera.c - Implementation of the camera routines
 * $Id: camera.c,v 1.5 1999-10-31 15:18:11-08 cmcmanis Exp cmcmanis $
 *
 * This module implements cameras in the Project: 3D Craft library.
 * 
 * A camera is the set of transforms that project the world on to
 * a 2D plane that is the screen. (Its a bit more complicated than
 * that but not much)
 *
 * The P3DC Viewing system uses a camera to do the screen rendition
 * of a scene from a particular viewpoint, called the view origin.
 * The world is transformed using the W2V transform to convert the
 * basic world co-ordinates into view co-ordinates. Both the world
 * and the view are left handed systems (this is atypical in graphics
 * library but I find it more convienient.) 
 *
 * Then the V2S transform partially transforms the view from view
 * space into screen space. I say partially because it actually
 * transforms the view homogenously so that I can do clipping using
 * the Sutherland-Hodgeman technique. (see clip.c). 
 *
 * Finally the camera contains a "view port" that is used in conjunction
 * with the final transformation to go from 4D homogenous co-ordinates
 * into 2D screen co-ordinates. 
 *
 * Change History:
 *	$Log: camera.c,v $
 *	Revision 1.5  1999-10-31 15:18:11-08  cmcmanis
 *	Additional camera fixes, now we have one attribute setting function with
 *	variable arguments.
 *
 *	Revision 1.4  1999-10-31 11:24:04-08  cmcmanis
 *	Camera rewrite is nearly complete. This seems like a logical
 *	milestone
 *
 *	Revision 1.3  1999/09/12 06:30:57  cmcmanis
 *	Rewrote clipping api (again?)
 *
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
 *	Renaming is complete, ModelTest runs again, the view was added to 
 *	support Models sharing the same draw functions as other programs.
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
#include <stdarg.h>
#include <math.h>
#include "p3dc.h"

extern void p3dc_camera_xfrm(p3dc_XFRM *, p3dc_PNT3 *, p3dc_PNT3 *, p3dc_PNT3 *, p3dc_FLOAT);
/* this function keeps track of the next available transform ID */
extern int __p3dc_get_xform_id(void);


#define setview(camera) {\
	p3dc_mult_xfrm(&((camera)->view_xfrm), &((camera)->V2S), &((camera)->W2V)); \
}

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
__p3dc_compute_P(p3dc_CMRA *cam) {
	p3dc_FLOAT fov = cam->fov;
	p3dc_FLOAT oofmd;
	p3dc_VIEW *view = &(cam->vp);
	p3dc_XFRM *p = &(cam->V2S);

	/* It starts out getting set to the identity transform */
	p3dc_identity_xfrm(p);
	/* pre-compute 1/(f - d) */
	oofmd = 1.0f / (cam->far_p - cam->near_p);

	/* Field of view (0 < fov < 180)  = tangent of 1/2 the field */
	fov = 1.0f / (p3dc_FLOAT) tan(D2R(cam->fov/2.0f));

	/* configure our transform */
	p->data[0][0] = fov * view->scale_y/view->scale_x;
	p->data[1][1] = fov;
	p->data[3][3] = 0;
	p->data[3][2] = 1.0f;
	p->data[2][2] = oofmd;
	p->data[2][3] = (p3dc_FLOAT) - cam->near_p * oofmd;
	p->id = __p3dc_get_xform_id();
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

	v = &(cam->W2V);

	/* 
	 * Start by computing our "z" vector. This is defined to
	 * be the difference between where we are (origin) and
	 * the point at which we are looking. (look)
	 */

	nz.x = cam->look.x - cam->origin.x;
	nz.y = cam->look.y - cam->origin.y;
	nz.z = cam->look.z - cam->origin.z;

	p3dc_camera_xfrm(v, &nz, &(cam->origin), &(cam->up), cam->twist);
}

/*
 * The generic initializer for the W2V and V2S transforms.
 */
static void
__p3dc_init_view(p3dc_CMRA *cam) {
	__p3dc_compute_V(cam);
	__p3dc_compute_P(cam);
	setview(cam);
}

/* 
 * Initialize a camera with a position looking at the origin.
 */
void
p3dc_init_camera(p3dc_CMRA *cam, p3dc_FLOAT fov, p3dc_FLOAT x, p3dc_FLOAT y, p3dc_FLOAT z) {

	/* Set the origin as specified */
	cam->origin.x = x;	cam->origin.y = y; 	cam->origin.z = z;

	/* By default, the world Y axis is "up" */
	cam->up.x = 0; cam->up.y = 1.0; cam->up.z = 0;

	/* Default is a 90 degree field of view */
	cam->fov = fov;

	/* The near plane is "1.0" unit in front of the camera origin */
	cam->near_p = 1.0;

	/* Camera's "far" plane is 512 units away */
	cam->far_p = 512.0;

	/* The camera's "look-at" reference point is the origin of the world */
	cam->look.x = cam->look.y = cam->look.z = 0;

	/* Camera twist is zero */
	cam->twist = 0;

	/* Viewport is full scale, centered on the window */
	cam->vp.scale_y = 240; cam->vp.scale_x = 320;
	cam->vp.center_x = 320; cam->vp.center_y = 240;

	/* Initialize the world and screen transforms and we're done */
	p3dc_identity_xfrm(&(cam->W2V));
	__p3dc_init_view(cam);
}

/*
 * Allocate and initialize a new camera.
 */
p3dc_CMRA *
p3dc_new_camera(p3dc_FLOAT fov, p3dc_FLOAT x, p3dc_FLOAT y, p3dc_FLOAT z) {
	p3dc_CMRA *res;

	res = (p3dc_CMRA *)calloc(1, sizeof(p3dc_CMRA));
	p3dc_init_camera(res, fov, x, y, z);
	return res;
}

/*
 * Set the camera's field of view. 
 */
void
p3dc_set_camera_fov(p3dc_CMRA *cam, p3dc_FLOAT fov) {
	cam->fov = fov;
	__p3dc_compute_P(cam);
	setview(cam);
}

/*
 * Position the camera somewhere in world space.
 */
void
p3dc_set_camera_origin(p3dc_CMRA *cam, p3dc_FLOAT x, p3dc_FLOAT y, p3dc_FLOAT z) {
	cam->origin.x = x;
	cam->origin.y = y;
	cam->origin.z = z;
	__p3dc_compute_V(cam);
	setview(cam);
}

/* 
 * Point the camera at another particular point in the world space. This
 * is useful, when you are tracking another object.
 */
void
p3dc_set_camera_target(p3dc_CMRA *cam, p3dc_FLOAT x, p3dc_FLOAT y, p3dc_FLOAT z, p3dc_PNT3 *up, p3dc_FLOAT twist) {
	cam->look.x = x;
	cam->look.y = y;
	cam->look.z = z;
	if (up)
		cam->up = *up;
	cam->twist = twist;
	__p3dc_compute_V(cam);
	setview(cam);
}

/*
 * This function "points" the camera in a particular direction in 
 * world space, so to look along the Y axis you would pass 0,1,0 as
 * the direction vector.
 */
void
p3dc_set_camera_dir(p3dc_CMRA *cam, p3dc_PNT3 *newZ, p3dc_PNT3 *up, p3dc_FLOAT twist) {

	cam->look.x = cam->origin.x + newZ->x;
	cam->look.y = cam->origin.y + newZ->y;
	cam->look.z = cam->origin.z + newZ->z;
	if (up)
		cam->up = *up;
	cam->twist = twist;
	__p3dc_compute_V(cam);
	setview(cam);
}

/*
 * This changes the viewport of the camera to be x pixels wide and y pixels
 * tall.
 */
void
p3dc_set_camera_window(p3dc_CMRA *cam, int x, int y) {
	cam->vp.scale_y = (float) (y/2.0f);
	cam->vp.scale_x = cam->vp.scale_y *  (float)(x) / (float)(y);
	cam->vp.center_x = (p3dc_FLOAT)((p3dc_FLOAT)x/2.0f);
	cam->vp.center_y = (p3dc_FLOAT)((p3dc_FLOAT)y/2.0f);
}

/*
 * This function sets the two corners of the camera window on the
 * screen. Not very happy with either of these.
 */
void
p3dc_set_camera_frame(p3dc_CMRA *cam, int lx, int ly, int ux, int uy) {
	int x, y;

	x = ux - lx;
	y = uy - ly;

	cam->vp.scale_y = (float) y / 2.0f;
	cam->vp.scale_x = (float) x / 2.0f;
	cam->vp.center_x = (float) (lx + x/2);
	cam->vp.center_y = (float) (ly + y/2);
	__p3dc_compute_P(cam);
	setview(cam);
}

/* XFRM2VECTOR(p3dc_XFRM *T, p3dc_PNT3 *x, p3dc_PNT3 *y, p3dc_PNT3 *z) */

#define XFRM2VECTOR(T, Vx, Vy, Vz) \
{ \
	(Vx)->x = (T)->data[0][0]; (Vx)->y = (T)->data[0][1]; (Vx)->z = (T)->data[0][2]; \
	(Vy)->x = (T)->data[1][0]; (Vy)->y = (T)->data[1][1]; (Vy)->z = (T)->data[1][2]; \
	(Vz)->x = (T)->data[2][0]; (Vz)->y = (T)->data[2][1]; (Vz)->z = (T)->data[2][2]; \
}

/* VECTOR2XFRM(p3dc_XFRM *T, p3dc_PNT3 *x, p3dc_PNT3 *y, p3dc_PNT3 *z) */
#define VECTOR2XFRM(T, Vx, Vy, Vz) \
{ \
	(T)->data[0][0] = (Vx)->x; (T)->data[0][1] = (Vx)->y; (T)->data[0][2] = (Vx)->z; \
	(T)->data[1][0] = (Vy)->x; (T)->data[1][1] = (Vy)->y; (T)->data[1][2] = (Vy)->z; \
	(T)->data[2][0] = (Vz)->x; (T)->data[2][1] = (Vz)->y; (T)->data[2][2] = (Vz)->z; \
	(T)->id = __p3dc_get_xform_id(); \
}

/*
 * Compute what the origin will be by inverse transforming
 * it through the 3 x 3 matrix in the upper left corner
 * of the new view.
 */
static void
new_origin(p3dc_XFRM *T, p3dc_PNT3 *old_origin) {
		/*
	 * Finally, instead of using the origin directly we have
	 * to translate it out of world space into camera space
	 */
	T->data[0][3] = -old_origin->x * T->data[0][0] + 
						-old_origin->y * T->data[0][1] +
							-old_origin->z * T->data[0][2];
	T->data[1][3] = -old_origin->x * T->data[1][0] +
						-old_origin->y * T->data[1][1] +
							-old_origin->z * T->data[1][2];
	T->data[2][3] = -old_origin->x * T->data[2][0] +
						-old_origin->y * T->data[2][1] +
							-old_origin->z * T->data[2][2];
	T->data[3][3] = 1.0f;
	T->id = __p3dc_get_xform_id();
}

/*
 * Roll the camera some relative number of degrees
 * from its current orientation.
 *
 * A "roll" is considered to be a rotation about the
 * Z axis which is exactly co-linear with the view,
 * thus rolling the camera causes the view of the
 * world to appear to "roll over."
 *
 * A positive roll
 * will turn the world counter clockwise in the view
 * (clockwise camera rotation...)
 *
 * Note that the roll is relative to the current 
 * position and not absolute! Positve angles roll
 * the camera clockwise and negative angles roll
 * the camera anti-clockwise.
 */
void
p3dc_set_camera_roll(p3dc_CMRA *cam, p3dc_FLOAT ang) {
	p3dc_XFRM N;

	p3dc_identity_xfrm(&N);
	N.data[0][0] = (p3dc_FLOAT)cos(D2R(ang));
	N.data[0][1] = - (p3dc_FLOAT)sin(D2R(ang));
	N.data[1][0] = - N.data[0][1]; 
	N.data[1][1] = N.data[0][0];
	p3dc_mult_xfrm(&(cam->W2V), &N, &(cam->W2V));
	new_origin(&(cam->W2V), &(cam->origin));
	/* recompute the view transform */
	setview(cam);
}

/*
 * Rotate the camera about the view point's Y
 * (or UP) access. This causes the camera to
 * turn right (positive angle) or left (negative
 * angle) to the direction it is looking.
 */
void
p3dc_set_camera_yaw(p3dc_CMRA *cam, p3dc_FLOAT ang) {
	p3dc_XFRM N;

	p3dc_identity_xfrm(&N);
	N.data[0][0] = (p3dc_FLOAT)cos(D2R(-ang));
	N.data[0][2] = (p3dc_FLOAT)sin(D2R(-ang));
	N.data[2][0] = - N.data[0][2]; 
	N.data[2][2] = N.data[0][0];
	p3dc_mult_xfrm(&(cam->W2V), &N, &(cam->W2V));
	new_origin(&(cam->W2V), &(cam->origin));
	/* recompute the view transform */
	setview(cam);
}

/*
 * Rotate the camera about the view point's X
 * axis. This causes the camera to look "up"
 * (negative angle) or "down" (postitive angle)
 * by some number of degrees.
 */
void
p3dc_set_camera_pitch(p3dc_CMRA *cam, p3dc_FLOAT ang) {
	p3dc_XFRM N;

	p3dc_identity_xfrm(&N);
	N.data[1][1] = (p3dc_FLOAT)cos(D2R(ang));
	N.data[1][2] = -(p3dc_FLOAT)sin(D2R(ang));
	N.data[2][1] = - N.data[1][2]; 
	N.data[2][2] = N.data[1][1];
	p3dc_mult_xfrm(&(cam->W2V), &N, &(cam->W2V));
	new_origin(&(cam->W2V), &(cam->origin));
	/* recompute the view transform */
	setview(cam);
}

/*
 * These two functions are used for playing around with 
 * camera algorithms. One can use them to experiment
 * and learn about the effects of various parameters
 * on the camera view.
 */
void
p3dc_set_camera_view(p3dc_CMRA *cam, p3dc_XFRM *new_view) {
	memcpy(&(cam->W2V), new_view, sizeof(p3dc_XFRM));
	cam->view_xfrm.id = __p3dc_get_xform_id();
	setview(cam);
}

void
p3dc_set_camera_screen(p3dc_CMRA *cam, p3dc_XFRM *new_pers) {
	memcpy(&(cam->V2S), new_pers, sizeof(p3dc_XFRM));
	cam->V2S.id = __p3dc_get_xform_id();
	setview(cam);
}

/*
 * Camera control
 * 
 * This function provides generalized access to controlling camera
 * parameters. I wrote it because I kept creating variations (set up and angle
 * set roll and pitch, etc) so it seems that the library really
 * needs a generalized version so that I can do what ever I want
 * without writing any new code. 
 *
 * It does use a fairly gross X11 hack of parameter/value lists but it was
 * that or try to remember the order of the parameters all the time. Yuck.
 *
 */
int
p3dc_set_camera(p3dc_CMRA *cam, int parm, ...) {
	va_list	vl;
	p3dc_XFRM N, *x;
	p3dc_PNT3 *tp3;
	p3dc_FLOAT *ang;
	int err = 0, needP = 0, needV = 0;

	va_start(vl, parm);
	while ((err == 0) && (parm != 0)) {
		switch (parm) {
		default: break;
		case P3DC_CAMERA_VIEW:
			x = va_arg(vl, p3dc_XFRM *);
			if (x == NULL) {
				err++;
				break;
			}
			memcpy(&(cam->W2V), x, sizeof(p3dc_XFRM));
			cam->W2V.id = __p3dc_get_xform_id();
			needV = 0;
			break;

		case P3DC_CAMERA_SCREEN:
			x = va_arg(vl, p3dc_XFRM *);
			if (x == NULL) {
				err++;
				break;
			}
			memcpy(&(cam->V2S), x, sizeof(p3dc_XFRM));
			cam->V2S.id = __p3dc_get_xform_id();
			needP = 0;
			break;

		case P3DC_CAMERA_YAW:
			ang = va_arg(vl, p3dc_FLOAT *);
			if (needV) {
				__p3dc_compute_V(cam);
			}
			p3dc_identity_xfrm(&N);
			N.data[0][0] = (p3dc_FLOAT)cos(D2R(-(*ang)));
			N.data[0][2] = (p3dc_FLOAT)sin(D2R(-(*ang)));
			N.data[2][0] = - N.data[0][2]; 
			N.data[2][2] = N.data[0][0];
			p3dc_mult_xfrm(&(cam->W2V), &N, &(cam->W2V));
			new_origin(&(cam->W2V), &(cam->origin));	
			needV = 0;
			break;

		case P3DC_CAMERA_PITCH:
			ang = va_arg(vl, p3dc_FLOAT *);
			if (needV) {
				__p3dc_compute_V(cam);
				needV = 0;
			}
			p3dc_identity_xfrm(&N);
			N.data[1][1] = (p3dc_FLOAT)cos(D2R(*ang));
			N.data[1][2] = -(p3dc_FLOAT)sin(D2R(*ang));
			N.data[2][1] = - N.data[1][2]; 
			N.data[2][2] = N.data[1][1];
			p3dc_mult_xfrm(&(cam->W2V), &N, &(cam->W2V));
			new_origin(&(cam->W2V), &(cam->origin));
			needV = 0;
			break;

		case P3DC_CAMERA_ROLL:
			ang = va_arg(vl, p3dc_FLOAT *);
			if (needV) {
				__p3dc_compute_V(cam);
				needV = 0;
			}
			p3dc_identity_xfrm(&N);
			N.data[0][0] = (p3dc_FLOAT)cos(D2R(*ang));
			N.data[0][1] = - (p3dc_FLOAT)sin(D2R(*ang));
			N.data[1][0] = - N.data[0][1]; 
			N.data[1][1] = N.data[0][0];
			p3dc_mult_xfrm(&(cam->W2V), &N, &(cam->W2V));
			new_origin(&(cam->W2V), &(cam->origin));
			break;

		case P3DC_CAMERA_FOV:
			ang = va_arg(vl, p3dc_FLOAT *);
			cam->fov = *ang;
			needP++;
			break;

		case P3DC_CAMERA_UP:
			tp3 = va_arg(vl, p3dc_PNT3 *);
			if (tp3 == NULL) {
				err++;
				break;
			}
			cam->up = *tp3;
			needV++;
			break;

		case P3DC_CAMERA_FAR:
			cam->far_p = va_arg(vl, p3dc_FLOAT);
			needP++;
			break;

		case P3DC_CAMERA_NEAR:
			cam->near_p = va_arg(vl, p3dc_FLOAT);
			needP++;
			break;

		case P3DC_CAMERA_TWIST:
			cam->twist = va_arg(vl, p3dc_FLOAT);
			needV++;
			break;

		case P3DC_CAMERA_ORIGIN:
			tp3 = va_arg(vl, p3dc_PNT3 *);
			if (tp3 == NULL) {
				err++;
				break;
			}
			cam->origin = *tp3;
			needV++;
			break;

		case P3DC_CAMERA_LOOK:
			tp3 = va_arg(vl, p3dc_PNT3 *);
			if (tp3 == NULL) {
				err++;
				break;
			}
			cam->look = *tp3;
			needV++;
			break;

		case P3DC_CAMERA_DIR:
			tp3 = va_arg(vl, p3dc_PNT3 *);
			if (tp3 == NULL) {
				err++;
				break;
			}
			cam->look.x = cam->origin.x + tp3->x;
			cam->look.y = cam->origin.y + tp3->y;
			cam->look.z = cam->origin.z + tp3->z;
			needV++;
			break;

		}
		parm = va_arg(vl, int);
	}
	if (needP)
		__p3dc_compute_P(cam);
	if (needV) 
		__p3dc_compute_V(cam);
	setview(cam);
	va_end(vl);
	return (err != 0);
}
