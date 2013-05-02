/*
 * Test 02 - create a point in three space, transform it to view
 * space, and verify that it is correctly lit.
 */

#include <stdio.h>
#include <math.h>
#include "g3d.h"


void
main(int argc, char *argv[]) {
	g3d_CMRA	*cam = g3d_new_camera(35, 10, 10, -10);
	g3d_VRTX *pt = g3d_new_vrtx(-10, -10, 10);
	g3d_VRTX *l[4];

	int i;

	l[0] = g3d_new_vrtx(0, -20, 0);
	l[1] = g3d_new_vrtx(-20, -20, 0);
	g3d_light_init();
	g3d_light_point(cam, pt);
	printf("Point lighting is ( %10.5f, %10.5f, %10.5f)\n", 
						pt->light.x, pt->light.y, pt->light.z);
	g3d_light_line(cam, l[0], l[1]);
	printf("Line lighting is (pt1) ( %10.5f, %10.5f, %10.5f)\n", 
						l[0]->light.x, l[0]->light.y, l[0]->light.z);
	printf("Line lighting is (pt2) ( %10.5f, %10.5f, %10.5f)\n", 
						l[1]->light.x, l[1]->light.y, l[1]->light.z);

	exit(0);
}

