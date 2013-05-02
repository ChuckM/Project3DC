/*
 * Test 01 - create a point in three space, transform it to view
 * space, and verify that it is on the Z axis
 */

#include <stdio.h>
#include <math.h>
#include "p3dc.h"

void
main(int argc, char *argv[]) {
	p3dc_VRTX	*v1;
	p3dc_CMRA	*cam = p3dc_new_camera(35, -10, -10, -10);

	v1 = p3dc_new_vrtx(0, 10, 10);
	p3dc_xfrm_vrtx(NULL, &(cam->world_xfrm), v1);
	printf("Original vertex is [%10.5f, %10.5f, %10.5f]\n", 
												v1->v.x, v1->v.y, v1->v.z);
	printf(" Transformed it is [%10.9f, %10.9f, %10.5f]\n", 
												v1->tv.x, v1->tv.y, v1->tv.z);
	p3dc_roll_camera(cam, NULL, 90);
	p3dc_xfrm_vrtx(NULL, &(cam->world_xfrm), v1);
	printf("Original vertex is [%10.5f, %10.5f, %10.5f]\n", 
												v1->v.x, v1->v.y, v1->v.z);
	printf(" Transformed it is [%10.9f, %10.9f, %10.5f]\n", 
												v1->tv.x, v1->tv.y, v1->tv.z);
	exit(0);
}

