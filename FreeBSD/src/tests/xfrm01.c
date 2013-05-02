/*
 * This code tests out the various transforms to make sure they work
 * correctly.
 */

#include <stdio.h>
#include <math.h>
#include <p3dc.h>

float
len(p3dc_VRTX *v) {
	p3dc_PNT3 a;

	a.x = v->tv.x; a.y = v->tv.y; a.z = v->tv.z;
	return (float)(sqrt(p3dc_dot_pnt3(&a, &a)));
}

/*
 * Generate string containing Vertex transformation and
 * vector lengths.
 */
static char *
pv(p3dc_VRTX *v, p3dc_VRTX *tv) {
	static char buf[1024];
	p3dc_PNT3 a;
	float l1, l2;
	
	a.x = v->v.x; a.y = v->v.y; a.z = v->v.z;
	l1 =  (float)(sqrt(p3dc_dot_pnt3(&a, &a)));

	a.x = tv->tv.x; a.y = tv->tv.y; a.z = tv->tv.z;
	l2 =  (float)(sqrt(p3dc_dot_pnt3(&a, &a)));

	snprintf(buf, 1024, "[%g, %g, %g] (%g) => [%g, %g, %g] (%g)",
			v->v.x, v->v.y, v->v.z, l1, tv->tv.x, tv->tv.y, tv->tv.z, l2);

	return buf;
}


void
main(int argc, char *argv[]) {
	p3dc_XFRM *X, *Y, *Z;
	p3dc_XFRM R;
	p3dc_VRTX *v = p3dc_new_vrtx(1.0f, 1.0f, 1.0f);
	p3dc_VRTX v2;

	/* 
 	 * The test methodology is as follows:
	 * 	The vector points 45 degres up and to the right
	 *  in the world. If we rotate it 45 degrees on any
	 *  single axis one axis will go to zero and the
	 *  other two will get sqrt(2) in them.
	 */
	X = p3dc_xrot_xfrm(NULL, 45);	
	Y = p3dc_yrot_xfrm(NULL, 45);
	Z = p3dc_zrot_xfrm(NULL, 45);
	printf("Initial vertex : [%g, %g, %g]\n", v->v.x, v->v.y, v->v.z);
	p3dc_xfrm_vrtx(NULL, X, v);
	printf("  Vertex transformed in X => [%g, %g, %g] (len=%g)\n", 
						v->tv.x, v->tv.y, v->tv.z, len(v));
	printf("PV %s\n", pv(v, v));
	p3dc_xfrm_vrtx(NULL, Y, v);
	printf("  Vertex transformed in Y => [%g, %g, %g] (len=%g)\n", 
						v->tv.x, v->tv.y, v->tv.z, len(v));
	printf("PV %s\n", pv(v, v));
	p3dc_xfrm_vrtx(NULL, Z, v);
	printf("  Vertex transformed in Z => [%g, %g, %g] (len=%g)\n", 
						v->tv.x, v->tv.y, v->tv.z, len(v));
	printf("PV %s\n", pv(v, v));

	/*
	 * Test Block #1
 	 * 	Simple rotations about a single Axis
	 */
	printf("Test #1: Simple 90 degree rotations\n");
	v->v.x = 0; v->v.y = 1.0; v->v.z = 0;
	printf("    Initial vertex : [%g, %g, %g]\n", v->v.x, v->v.y, v->v.z);

	p3dc_identity_xfrm(X); p3dc_xrot_xfrm(X, 90);
	p3dc_identity_xfrm(Y); p3dc_yrot_xfrm(Y, 90);
	p3dc_identity_xfrm(Z); p3dc_zrot_xfrm(Z, 90);
	p3dc_xfrm_vrtx(NULL, X, v);
	printf("    +90 degrees on X : %s\n", pv(v, v));
	p3dc_xfrm_vrtx(NULL, Y, v);
	printf("    +90 degrees on Y : %s\n", pv(v, v));
	p3dc_xfrm_vrtx(NULL, Z, v);
	printf("    +90 degrees on Z : %s\n", pv(v, v));

	/*
	 * Test Block #2
	 *	Rotations about two AXIS (checked)
	 */
	printf("\nTest #2: Dual Axis Rotations\n");
	v->v.x = 0; v->v.y = 1.0; v->v.z = 0;
	printf("    Initial vertex : [%g, %g, %g]\n", v->v.x, v->v.y, v->v.z);
	
	p3dc_identity_xfrm(&R);
	p3dc_mult_xfrm(&R, Y, X);
	p3dc_xfrm_vrtx(&v2, X, v);
	p3dc_xfrm_vrtx(NULL, Y, &v2);
	p3dc_xfrm_vrtx(NULL, &R, v);
	printf("          +90 X then Y done with R : %s\n", pv(v, v));
	printf("    +90 X then Y done individually : %s\n", pv(v, &v2));
	
	exit(0);
}

