/*
 * Second set of transform tests
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <p3dc.h>

void
show_vertex(p3dc_VRTX *v, char *id) {
	printf("[%s] :: [%6.3f, %6.3f, %6.3f] => ", id,
			v->v.x, v->v.y, v->v.z);
	printf("[%6.3f, %6.3f, %6.3f, %6.3f]\n",
			v->tv.x, v->tv.y, v->tv.z, v->tv.w);
}


void
main(int argc, char *argv[]) {
	p3dc_XFRM	T;
	p3dc_VRTX	*v;


	printf("When looking along (from - infinity to + infinity) rotations\n");
	printf("are ANTI-CLOCKWISE about the axis for positive values and \n");
	printf("CLOCKWISE when the values are negative.\n");

	v = p3dc_new_vrtx(1.0f, 0.0f, 0.0f);
	p3dc_identity_xfrm(&T);
	p3dc_zrot_xfrm(&T, 45);
	p3dc_xfrm_vrtx(NULL, &T, v);
	show_vertex(v, "45 deg on Z");

	v->v.x = 0; v->v.y = 1.0f;
	p3dc_identity_xfrm(&T);
	p3dc_xrot_xfrm(&T, 45);
	p3dc_xfrm_vrtx(NULL, &T, v);
	show_vertex(v, "45 deg on X");

	v->v.y = 0; v->v.z = 1.0;
	p3dc_identity_xfrm(&T);
	p3dc_yrot_xfrm(&T, 45);
	p3dc_xfrm_vrtx(NULL, &T, v);
	show_vertex(v, "45 deg on Y");

	v->v.x = 1.0f; v->v.z = 0;
	p3dc_identity_xfrm(&T);
	p3dc_zrot_xfrm(&T, -45);
	p3dc_xfrm_vrtx(NULL, &T, v);
	show_vertex(v, "-45 deg on Z");

	v->v.x = 0; v->v.y = 1.0f;
	p3dc_identity_xfrm(&T);
	p3dc_xrot_xfrm(&T, -45);
	p3dc_xfrm_vrtx(NULL, &T, v);
	show_vertex(v, "-45 deg on X");

	v->v.y = 0; v->v.z = 1.0;
	p3dc_identity_xfrm(&T);
	p3dc_yrot_xfrm(&T, -45);
	p3dc_xfrm_vrtx(NULL, &T, v);
	show_vertex(v, "-45 deg on Y");
	printf("\n=========================\n");
	printf("Now stepping through a simple view transform\n");
	v = p3dc_new_vrtx(1.0f, 1.0f, 1.0f);

	p3dc_identity_xfrm(&T);
	p3dc_locate_xfrm(&T, -2.0f, -2.0f, -2.0f);
	p3dc_xfrm_vrtx(NULL, &T, v);
	show_vertex(v, "xlate to (2,2,2) ");
	
	p3dc_zrot_xfrm(&T, -45);
	p3dc_xfrm_vrtx(NULL, &T, v);
	show_vertex(v, "+z rotation -45");

	p3dc_yrot_xfrm(&T, 180.0f-54.74f);
	p3dc_xfrm_vrtx(NULL, &T, v);
	show_vertex(v, "+y rot (180-54.74)");

}
