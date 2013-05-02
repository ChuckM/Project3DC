/*
 * SimpleViewer.c
 *
 * This file implements the first two viewing systems from
 * Watt's "3D Computer Graphics". They assist in understanding
 * the relationship between field of view, camera view, etc.
 *
 * $Id:$
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <p3dc.h>

extern int __p3dc_get_xform_id();
extern void p3dc_dump_xfrm();

void
show_vertex(p3dc_VRTX *v, char *id) {
	printf("[%s] :: [%6.3f, %6.3f, %6.3f] => ", id,
			v->v.x, v->v.y, v->v.z);
	printf("[%6.3f, %6.3f, %6.3f, %6.3f]\n",
			v->tv.x, v->tv.y, v->tv.z, v->tv.w);
}

void
show_transform(char *id, p3dc_XFRM *T, p3dc_VRTX *v) {
	printf("Transform with %s:\n", id);
	p3dc_dump_xfrm(T, NULL, NULL, NULL, NULL);
	p3dc_xfrm_vrtx(NULL, T, v);
	show_vertex(v, "Post XFRM");
}

/*
 * Main program.
 *
 * Construct a scene, view it, and wait to exit.
 */
void
main(int argc, char *argv[]) {
	p3dc_VIEW	vp;
	p3dc_XFRM	view, view2;
	p3dc_VRTX	*v;
	p3dc_FLOAT	theta, phi, r;


	/*
	 * In this application we are manually creating the
	 * viewport and the view rather than using the camera
	 * system so that simpler viewing systems can be
	 * implemented and played with.
	 */

	theta = 45.0;
	phi = 54.74;
	r = 5.0;

	v = p3dc_new_vrtx(1.0f, 1.0f,1.0f);

	vp.scale_x = 1.0f/240.0f;
	vp.scale_y = 1.0f/240.0f;
	vp.center_x = 320.0f;
	vp.center_y = 240.0f;
	p3dc_log_open("/dev/tty");

	p3dc_identity_xfrm(&view);
	p3dc_locate_xfrm(&view, 
		(float)(-r * cos(D2R(theta)) * sin(D2R(phi))),
		(float)(-r * sin(D2R(theta)) * sin(D2R(phi))),
		(float)(-r * cos(D2R(phi))));
	show_transform("Locate", &view, v);
	p3dc_zrot_xfrm(&view, -theta);
	show_transform("+Z rotations", &view, v);
	p3dc_xrot_xfrm(&view, 180-phi);
	show_transform("+X rotation", &view, v);
	p3dc_identity_xfrm(&view2);

	p3dc_identity_xfrm(&view2);

	view2.data[0][0] = (float)(-sin(D2R(theta)));
	view2.data[0][1] = (float)(cos(D2R(theta)));
	
	view2.data[1][0] = (float)(-cos(D2R(theta))*cos(D2R(phi)));
	view2.data[1][1] = (float)(-sin(D2R(theta))*cos(D2R(phi)));
	view2.data[1][2] = (float)(sin(D2R(phi)));

	view2.data[2][0] = (float)(-cos(D2R(theta))*sin(D2R(phi)));
	view2.data[2][1] = (float)(-sin(D2R(theta))*sin(D2R(phi)));
	view2.data[2][2] = (float)(-cos(D2R(phi)));
	view2.data[2][3] = r;

	view2.id = __p3dc_get_xform_id();

	printf("Transforming the origin using the view transform:\n");
	p3dc_dump_xfrm(&view, NULL, NULL, NULL, NULL);
	p3dc_xfrm_vrtx(NULL, &view, v);
	printf("Original vertex is [%10.5f, %10.5f, %10.5f]\n", 
												v->v.x, v->v.y, v->v.z);
	printf(" Transformed it is [%10.9f, %10.9f, %10.5f, %10.5f]\n", 
									v->tv.x, v->tv.y, v->tv.z, v->tv.w);

	printf("Transforming the origin using the view2 transform:\n");
	p3dc_dump_xfrm(&view2, NULL, NULL, NULL, NULL);
	p3dc_xfrm_vrtx(NULL, &view2, v);
	printf("Original vertex is [%10.5f, %10.5f, %10.5f]\n", 
												v->v.x, v->v.y, v->v.z);
	printf(" Transformed it is [%10.9f, %10.9f, %10.5f, %10.5f]\n", 
									v->tv.x, v->tv.y, v->tv.z, v->tv.w);

}
