/*
 * transform.c - Basic transform functions for P3DC
 * $Id: transform.c,v 1.2 1999/09/02 03:22:47 cmcmanis Exp cmcmanis $
 *
 * This module implements the transform functions for p3dc
 *
 * Change Log:
 *	$Log: transform.c,v $
 *	Revision 1.2  1999/09/02 03:22:47  cmcmanis
 *	The great LIST rewrite, lists are much cleaner now
 *
 *	Revision 1.1  1999/08/18 01:34:00  cmcmanis
 *	Initial revision
 *
 *	Revision 1.1  1999/08/18 01:06:22  cmcmanis
 *	Initial revision
 *
 *	Revision 1.7  1999/07/25 09:11:56  cmcmanis
 *	The great renaming from g3d/G3D -> p3dc/P3DC
 *
 *	Revision 1.6  1998/11/16 03:41:04  cmcmanis
 *	Updated clipping
 *
 *	Revision 1.6  1998/11/08 05:22:10  cmcmanis
 *	Outdated (renamed to 'H:\P3DC-RCS\src\lib\transform.c,v').
 *
 *	Revision 1.5  1998/11/05 20:23:46  cmcmanis
 *	Checkin prior to moving into a better source tree structure
 *
 *	Revision 1.4  1998/10/18 09:29:47  cmcmanis
 *	Models now position and move correctly. I've re-ordered my matrices
 *	to be SPHIGs compatible so that I can use SPHIGS to test my results.
 *	Created p3dc_camera_xfrm and p3dc_orientation_xfrm as compliments of
 *	each other. Cool stuff there.
 *
 *	Revision 1.3  1998/10/10 07:00:45  cmcmanis
 *	Model parsing is written (not fully debugged) and the texture structures
 *	are closer to their final form. By the next check in we should have a
 *	few simple models up and running with textured models to follow.
 *
 *	Revision 1.2  1998/09/04 05:47:54  cmcmanis
 *	Lighting works for lines and points.
 *	New memory allocator is in place but not
 *	heavily used yet.
 *
 *	Revision 1.1  1998/09/02 08:48:26  cmcmanis
 *	Initial revision
 *
 *	Revision 1.3  1998/08/27 07:56:36  cmcmanis
 *	New clip architecture, added the NLST allocator/deallocator
 *
 *	Revision 1.2  1998/08/25 16:12:53  cmcmanis
 *	Basic compile working, built with new p3dc.h.
 *
 *	Revision 1.1  1998/08/24 16:36:02  cmcmanis
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

static unsigned short xform_id = 2;

static p3dc_XFRM *p3dc_new_xfrm();

/* 
 * Returns and/or initializes a transform to the identity transform.
 */
p3dc_XFRM *
p3dc_identity_xfrm(p3dc_XFRM *tr) {
	if (tr == NULL)
		tr = p3dc_new_xfrm();

	memset((char *)tr, 0, sizeof(p3dc_XFRM));
	tr->data[0][0] = tr->data[1][1] = tr->data[2][2] = tr->data[3][3] = 1.0f;
	tr->id = 1; /* special identity xid */
	return tr;
}

/*
 * Return a new transform. This function pulls a transform from a
 * list of previously malloc'd transforms. If there are no free 
 * transforms available it mallocs a new one.
 */
static p3dc_XFRM *
p3dc_new_xfrm() {
	p3dc_XFRM *res;
	res = (p3dc_XFRM *)p3dc_new_type(P3DC_XFRM);
	res->id = 0; /* special id means "unitialized" */
	return res;
}

/*
 * Puts a used (but no longer needed) transform on the free pile. Note
 * that this routine ASSUMES that the transform was allocated using
 * new_xfrm above.
 */
void
p3dc_free_xfrm(p3dc_XFRM **t) {
	p3dc_free_type(*t);
	*t = NULL;
}

/*
 * This function does 'a = a * b'
 */
p3dc_XFRM * 
p3dc_mult_xfrm(p3dc_XFRM *res, p3dc_XFRM *a, p3dc_XFRM *b) {
	int i, j;
	p3dc_XFRM tmp;
	if (res == NULL)
		res = p3dc_new_xfrm();

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			p3dc_FLOAT v =
					a->data[i][0] * b->data[0][j] +
					a->data[i][1] * b->data[1][j] +
					a->data[i][2] * b->data[2][j] +
					a->data[i][3] * b->data[3][j];
			tmp.data[i][j] = v;
		}
	}
	memcpy((char *)res, (char *)&tmp, sizeof(tmp));
	res->id = xform_id++;
	return res;
}

/*
 * Prints a transform to the log file. Useful for debugging. It is
 * Also similar to the SPHIGS format so that I can compare my transforms
 * to theirs easily.
 */
void
p3dc_dump_xfrm(p3dc_XFRM *t, char *title, char *head, char *format, char *tail) {
	int i, j;

	if (title  == NULL)
		title  = "P3DC matrix (XID = %d):\n";
	if (head   == NULL)
		head   = "  ";
	if (format == NULL)
		format = "%#8.4lf  ";
	if (tail   == NULL)
		tail   = "\n";

	p3dc_log(title, t->id);

	for (i = 0; i < 4; i++) {
		p3dc_log(head);
		for (j = 0; j < 4; j++)
			p3dc_log(format, t->data[i][j]);
		p3dc_log(tail);
	}
}

/*
 * This function sets a translation for the transform
 */
p3dc_XFRM *
p3dc_locate_xfrm(p3dc_XFRM *t, p3dc_FLOAT x, p3dc_FLOAT y, p3dc_FLOAT z) {
	p3dc_XFRM res;
	if (t == NULL) {
		t = p3dc_identity_xfrm(NULL);
		t->data[0][3] = x;
		t->data[1][3] = y;
		t->data[2][3] = z;
		t->id = xform_id++;
		return t;
	}
	
	p3dc_identity_xfrm(&res);
	res.data[0][3] = x;
	res.data[1][3] = y;
	res.data[2][3] = z;
	p3dc_mult_xfrm(t, &res, t);
	return t;
}

/*
 * This function creates a translation that is the linear
 * composition of a vector and a distance.
 */
p3dc_XFRM *
p3dc_move_xfrm(p3dc_XFRM *t, p3dc_PNT3 *v, p3dc_FLOAT dt) {
	p3dc_XFRM res;
	if (t == NULL) {
		t = p3dc_identity_xfrm(NULL);
		t->data[0][3] = v->x * dt;
		t->data[1][3] = v->y * dt;
		t->data[2][3] = v->z * dt;
		t->id = xform_id++;
		return t;
	}
	
	p3dc_identity_xfrm(&res);
	res.data[0][3] = v->x * dt;
	res.data[1][3] = v->y * dt;
	res.data[2][3] = v->z * dt;
	p3dc_mult_xfrm(t, &res, t);
	return t;
}

/*
 * This function provides a non-uniform scaling transform non-uniformly. 
 */
p3dc_XFRM *
p3dc_scaleXYZ_xfrm(p3dc_XFRM *t, p3dc_FLOAT x, p3dc_FLOAT y, p3dc_FLOAT z) {
	p3dc_XFRM res;
	if (t == NULL) {
		t = p3dc_identity_xfrm(NULL);
		t->data[0][0] = x;
		t->data[1][1] = y;
		t->data[2][2] = z;
		t->id = xform_id++;
		return t;
	}
	
	p3dc_identity_xfrm(&res);
	res.data[0][0] = x;
	res.data[1][1] = y;
	res.data[2][2] = z;
	p3dc_mult_xfrm(t, &res, t);
	return t;
}

/*
 * Provide for uniform scaling of the transform
 */
p3dc_XFRM *
p3dc_scale_xfrm(p3dc_XFRM *t, p3dc_FLOAT size) {
	return (p3dc_scaleXYZ_xfrm(t, size, size, size));
}

/*
 * Provide for rotation about the Z axis.
 */
p3dc_XFRM * 
p3dc_zrot_xfrm(p3dc_XFRM *t, p3dc_FLOAT deg) {
	p3dc_FLOAT rad = deg * DTR;
	p3dc_XFRM T2;

	if (t == NULL) {
		t = p3dc_identity_xfrm(NULL);
		t->data[0][0] = (p3dc_FLOAT)cos(rad);
		t->data[0][1] = - (p3dc_FLOAT)sin(rad);
		t->data[1][0] = - t->data[1][0]; 
		t->data[1][1] = t->data[0][0];
		t->id = xform_id++;
	} else {
		p3dc_identity_xfrm(&T2);
		T2.data[0][0] = (p3dc_FLOAT)cos(rad);
		T2.data[0][1] = - (p3dc_FLOAT)sin(rad);
		T2.data[1][0] = - T2.data[1][0]; 
		T2.data[1][1] = T2.data[0][0];
		p3dc_mult_xfrm(t, &T2, t);
	}
	return t;
}

/*
 * This transform will rotate about the Y axis
 */
p3dc_XFRM * 
p3dc_yrot_xfrm(p3dc_XFRM *t, p3dc_FLOAT deg) {
	p3dc_FLOAT rad = deg * DTR;
	p3dc_XFRM T2;

	if (t == NULL) {
		t = p3dc_identity_xfrm(NULL);
		t->data[0][0] = (p3dc_FLOAT)cos(rad);
		t->data[0][2] = (p3dc_FLOAT)sin(rad);
		t->data[2][0] = - t->data[2][0]; 
		t->data[2][2] = t->data[0][0];
		t->id = xform_id++;
	} else {
		p3dc_identity_xfrm(&T2);
		T2.data[0][0] = (p3dc_FLOAT)cos(rad);
		T2.data[0][2] = (p3dc_FLOAT)sin(rad);
		T2.data[2][0] = - T2.data[2][0]; 
		T2.data[2][2] = T2.data[0][0];
		p3dc_mult_xfrm(t, &T2, t);
	}
	return t;
}

/*
 * This transform will rotate about the X axis
 */
p3dc_XFRM * 
p3dc_xrot_xfrm(p3dc_XFRM *t, p3dc_FLOAT deg) {
	p3dc_FLOAT rad = deg * DTR;
	p3dc_XFRM T2;

	if (t == NULL) {
		t = p3dc_identity_xfrm(NULL);
		t->data[1][1] = (p3dc_FLOAT)cos(rad);
		t->data[1][2] = -(p3dc_FLOAT)sin(rad);
		t->data[2][1] = - t->data[2][1]; 
		t->data[2][2] = t->data[1][1];
		t->id = xform_id++;
	} else {
		p3dc_identity_xfrm(&T2);
		T2.data[1][1] = (p3dc_FLOAT)cos(rad);
		T2.data[1][2] = -(p3dc_FLOAT)sin(rad);
		T2.data[2][1] = - T2.data[2][1]; 
		T2.data[2][2] = T2.data[1][1];
		p3dc_mult_xfrm(t, &T2, t);
	}
	return t;
}

/*
 * This is a slick piece of code, it uses a direction vector, an origin
 * and a 'roll' value to compute the necessary transformation matrix to
 * apply to something to get position it there, and in a particular 
 * orientation. 
 *
 * The code works as follows:
 *		1) Use direction to establish the basis vector 'z'.
 *		2) Use the world 'up' basis vector, crossed with 'z' to
 *		   produce the basis vector 'x'
 *		3) Cross the basis vector x and the basis vector z to get
 *		   the real 'y' basis vector.
 *		4) copy these into the upper 3 x 3 matrix of a 4 x 4 transform.
 *		5) Put the origin into the lower 4 x 1 matrix of a 4 x 4 transform.
 *		6) Rotate about the Z axis 'roll' degrees.
 *
 *	All done. 
 *
 * The results of apply this transform to a MODEL will transform the model
 * from MODEL space into WORLD space.
 */
p3dc_XFRM *
p3dc_orientation_xfrm(p3dc_XFRM *v, p3dc_PNT3 *dir, p3dc_PNT3 *orig, p3dc_FLOAT roll) {
    p3dc_PNT3 x, y;
	p3dc_PNT3 nx, ny, nz;

	if (v == NULL)
		v = p3dc_new_xfrm();

	/*
	 * These locations will be constants.
	 */
	v->data[3][0] = v->data[3][1] = v->data[3][2] = 0;
	v->data[3][3] = 1;

	/* 
	 * Z basis is the direction vector normalized...
	 */
	nz.x = dir->x;
	nz.y = dir->y;
	nz.z = dir->z;

	if (p3dc_normalize_pnt3(&nz)) { /* !! Zero length Z axis !! */
		nz.x = 0;
		nz.y = 0;
		nz.z = 1.0f;
	}

	/* X is presumed to be 'right' facing and Y is presumed to be 'up' */
	x.x = 1; x.y = 0; x.z = 0;
	y.x = 0; y.y = 1; y.z = 0;
	
	/* 
	 * Our new X vector is the cross product of Z and Y, unless 
	 * of course we've made the new Z the _same_ as Y. In which
	 * case we're screwed. (Gymbal Lock) In that case we'll use
	 * Z and old X to compute the new Y.
	 */
	p3dc_cross_pnt3(&y, &nz, &nx);

	/* !! Gymbal Lock (z and y are parallel) !! */
	if (p3dc_normalize_pnt3(&nx)) {
		/* Try to compute Y as (Z x X) */
		p3dc_cross_pnt3(&nz, &x, &ny);
		/* Note is can't be parallel so no need to check this for 0 */
		p3dc_normalize_pnt3(&ny);
		/* Now Y is a unit vector and we compute X as (Z x new Y) */
		p3dc_cross_pnt3(&ny, &nz, &nx);
		p3dc_normalize_pnt3(&nx);
		/* All done. */
	} else {
		/* Now compute Y as the cross product of (new Z x new X) */
		p3dc_cross_pnt3(&nz, &nx, &ny);
		p3dc_normalize_pnt3(&ny);
	}
	/* Now store it into the matrix as 3 basis vectors */
	v->data[0][0] = nx.x; v->data[0][1] = ny.x; v->data[0][2] = nz.x;
	v->data[1][0] = nx.y; v->data[1][1] = ny.y; v->data[1][2] = nz.y;
	v->data[2][0] = nx.z; v->data[2][1] = ny.z; v->data[2][2] = nz.z;

	/*
	 * Next we fill in the relocation of the origin appropriately.
	 */
	v->data[0][3] = orig->x; 
	v->data[1][3] = orig->y;
	v->data[2][3] = orig->z;

	/*
	 * If a camera roll was specified that is done now by using the
	 * computed X and Y vectors and then rotating them around the
	 * new Z axis.
	 */
	if (roll != 0) {
		p3dc_FLOAT si, cs;

		si = (p3dc_FLOAT)sin(roll * DTR);
		cs = (p3dc_FLOAT)cos(roll * DTR);
		nx.x = cs * v->data[0][0] - si * v->data[0][1];
		nx.y = cs * v->data[1][0] - si * v->data[1][1];
		nx.z = cs * v->data[2][0] - si * v->data[2][1];
		p3dc_normalize_pnt3(&nx);
		ny.x = cs * v->data[0][1] + si * v->data[0][0];
		ny.y = cs * v->data[1][1] + si * v->data[1][0];
		ny.z = cs * v->data[2][1] + si * v->data[2][0];
		p3dc_normalize_pnt3(&ny);
		v->data[0][0] = nx.x; v->data[0][1] = ny.x; 
		v->data[1][0] = nx.y; v->data[1][1] = ny.y;
		v->data[2][0] = nx.z; v->data[2][1] = ny.z;
	}

	/* Give it a unique transform ID */
	v->id = xform_id++;
	return v;
}

/*
 * This works exactly like the orientation matrix above except it
 * computes the INVERSE matrix. This is a bit trickier but by doing
 * it this way we don't have to do two steps to locate the camera
 * (orient it, compute the inverse). The result of applying the
 * world points with this matrix is that they will be converted
 * from the WORLD co-ordinate system into the VIEW (aka camera)
 * co-ordinate system.
 */
p3dc_XFRM *
p3dc_camera_xfrm(p3dc_XFRM *v, p3dc_PNT3 *dir, p3dc_PNT3 *orig, p3dc_FLOAT roll) {
    p3dc_PNT3 x, y;
	p3dc_PNT3 nx, ny, nz;

	if (v == NULL)
		v = p3dc_new_xfrm();

	v->data[0][3] = v->data[1][3] = v->data[2][3] = 0;

	/* 
	 * Z is again the direction vector.
	 */

	nz.x = dir->x;
	nz.y = dir->y;
	nz.z = dir->z;

	if (p3dc_normalize_pnt3(&nz)) { /* !! Zero length Z axis !! */
		nz.x = 0;
		nz.y = 0;
		nz.z = 1.0f;
	}

	y.x = 0; y.y = 1.0; y.z = 0;
	x.x = 1; x.y = 0; x.z = 0;
	
	/* 
	 * Our new X vector is the cross product of Z and Y, unless 
	 * of course we've made the new Z the _same_ as Y. In which
	 * case we're screwed. (Gymbal Lock) In that case we'll use
	 * Z and old X to compute the new Y.
	 */
	p3dc_cross_pnt3(&y, &nz, &nx);

	/* !! Gymbal Lock (z and y are parallel) !! */
	if (p3dc_normalize_pnt3(&nx)) {
		/* Try to compute Y as (Z x X) */
		p3dc_cross_pnt3(&nz, &x, &ny);
		/* Note is can't be parallel so no need to check this for 0 */
		p3dc_normalize_pnt3(&ny);
		/* Now Y is a unit vector and we compute X as (Z x new Y) */
		p3dc_cross_pnt3(&ny, &nz, &nx);
		p3dc_normalize_pnt3(&nx);
		/* All done. */
	} else {
		/* Now compute Y as the cross product of (new Z x new X) */
		p3dc_cross_pnt3(&nz, &nx, &ny);
		p3dc_normalize_pnt3(&ny);
	}

	/* 
	 * Now the first difference, store these basis vectors in
	 * column major form (versus row major form)
	 */
	v->data[0][0] = nx.x; v->data[1][0] = ny.x; v->data[2][0] = nz.x;
	v->data[0][1] = nx.y; v->data[1][1] = ny.y; v->data[2][1] = nz.y;
	v->data[0][2] = nx.z; v->data[1][2] = ny.z; v->data[2][2] = nz.z;

	/*
	 * Apply the roll, using the row major form.
	 */
	if (roll != 0) {
		p3dc_FLOAT si, cs;

		si = (p3dc_FLOAT)sin(roll * DTR);
		cs = (p3dc_FLOAT)cos(roll * DTR);
		nx.x = cs * v->data[0][0] - si * v->data[1][0];
		nx.y = cs * v->data[0][1] - si * v->data[1][1];
		nx.z = cs * v->data[0][2] - si * v->data[1][2];
		p3dc_normalize_pnt3(&nx);
		ny.x = cs * v->data[1][0] + si * v->data[0][0];
		ny.y = cs * v->data[1][1] + si * v->data[0][1];
		ny.z = cs * v->data[1][2] + si * v->data[0][2];
		p3dc_normalize_pnt3(&ny);
		v->data[0][0] = nx.x; v->data[1][0] = ny.x; 
		v->data[0][1] = nx.y; v->data[1][1] = ny.y;
		v->data[0][2] = nx.z; v->data[1][2] = ny.z;
	}

	/*
	 * Finally, instead of using the origin directly we have
	 * to translate it out of world space into camera space
	 */
	v->data[0][3] = -orig->x * nx.x + 
						-orig->y * nx.y +
							-orig->z * nx.z;
	v->data[1][3] = -orig->x * ny.x +
						-orig->y * ny.y +
							-orig->z * ny.z;
	v->data[2][3] = -orig->x * nz.x +
						-orig->y * nz.y +
							-orig->z * nz.z;
	v->data[3][3] = 1.0f;

	v->id = xform_id++;
	return v;
}

int
__p3dc_get_xform_id() {
	return xform_id++;
}
