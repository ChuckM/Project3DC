/*
 * Locate camera positions that clip the triangle as needed.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include "p3dc.h"

void
main(int argc, char *argv[]) {
	p3dc_MODEL *m;
	char	*name;
	float	x, y, z;
	float	mx, my, mz;
	p3dc_CMRA	*cam = p3dc_new_camera(36.0f, 0.0f, 0.0f, -16.0f);

	p3dc_log_open("tri.log");
	name = "tri";
	if (argc > 1) x = atof(argv[1]);
	else x = 0.0f;
	if (argc > 2) y = atof(argv[2]);
	else y = 0.0f;
	if (argc > 3) z = atof(argv[3]);
	else z = -10.0f;
	if (argc > 4) mx = atof(argv[4]);
	else mx = 0.0f;
	if (argc > 5) my = atof(argv[5]);
	else my = 0.0f;
	if (argc > 6) mz = atof(argv[6]);
	else mz = 0.0f;

	p3dc_log("TRI: Using model %s\n", name);
	m = p3dc_new_model(name, 1.0, 1.0, 1.0);
	if (m) {
		int i;

		p3dc_rbprint_list(&(m->parts));
		p3dc_dump_model(m); 
		p3dc_log("Model corners are: \n");
		for (i = 0; i < 8; i++) {
			p3dc_log("[%d] - (%g, %g, %g)\n", i+1, m->corners[i].v.x, 
								m->corners[i].v.y, m->corners[i].v.z);
		}
	} else  {
		p3dc_log("Didn't successfully read the model.\n");
		exit(1);
	}
	p3dc_locate_model(m, mx, my, mz);
	p3dc_open();
	p3dc_locate_camera(cam, x, y, z);
	p3dc_prepare_frame(&COLOR_black);
	printf("Camera @ (%g, %g, %g) and Model @ (%g, %g, %g)\n", x, y, z,
			mx, my, mz);
	p3dc_draw_axes(&(cam->vp), &(cam->view_xfrm), P3DC_DRAW_NOSHADE, 0.25f);
	p3dc_draw_model(&(cam->vp), &(cam->view_xfrm), P3DC_DRAW_NOSHADE, m);
	p3dc_flip_frame(1);
	p3dc_snapshot("tri.png", "Chuck McManis", NULL);
	p3dc_close();
	exit(0);
}
