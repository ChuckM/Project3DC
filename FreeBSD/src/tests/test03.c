#include <stdio.h>
#include <stdlib.h>
#include "g3d.h"

void
main(int argc, char *argv[]) {
	g3d_MODEL *cube = new_cube(0xff0000ff);
	g3d_CAMERA eye;

	g3d_init_camera(&eye, 10, 10, -25);
	g3d_open();
	g3d_prepare_frame(NULL);
	g3d_draw_model(&eye, cube, 3);
	g3d_flip_frame(1);
	g3d_close();
	exit(0);
}
