/*
 * Read a model and then draw it on the screen.
 */
#include <stdio.h>
#include <stdlib.h>
#include "p3dc.h"

void
main(int argc, char *argv[]) {
	p3dc_MODEL *m;
	char	*name;
	float	x, y, z;
	p3dc_CMRA	*camTop   = p3dc_new_camera(15.0f, 3.0, 35.0, 3.0);
	p3dc_CMRA	*camFront = p3dc_new_camera(15.0f, 3.0, 3.0, 35.0);
	p3dc_CMRA	*camRight = p3dc_new_camera(15.0f, 35.0, 3.0, 3.0);
	p3dc_CMRA	*camOrtho = p3dc_new_camera(36.0f, 10.0, 10.0, 10.0);

	p3dc_frame_camera(camTop, 0, 240, 320, 480);
	p3dc_point_camera(camTop, 0.0, -1.0, 0.0);
	p3dc_frame_camera(camFront, 0, 0, 320, 240);
	p3dc_point_camera(camFront, 0.0, 0.0, -1.0);
	p3dc_frame_camera(camRight, 320, 240, 640, 480);
	p3dc_point_camera(camRight, -1.0, 0, 0);
	p3dc_frame_camera(camOrtho, 320, 0, 640, 240);
	p3dc_log_open("test14.log");
	name = (argc > 1) ? argv[1] : "cube";
	if (argc > 2) x = atof(argv[2]);
	else x = 2.0f;
	if (argc > 3) y = atof(argv[3]);
	else y = 2.0f;
	if (argc > 4) z = atof(argv[4]);
	else z = -20.0f;

	p3dc_log("TEST14: Using model %s\n", name);
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
	p3dc_locate_model(m, 1.0f, 1.0f, 1.0f);
	p3dc_open();
	p3dc_prepare_frame(&COLOR_black);
	
	p3dc_draw_axes(&(camTop->vp), &(camTop->view_xfrm), P3DC_DRAW_NOSHADE, 0.25f);
	p3dc_draw_model(&(camTop->vp), &(camTop->view_xfrm), P3DC_DRAW_NOSHADE, m);

	p3dc_draw_axes(&(camFront->vp), &(camFront->view_xfrm), P3DC_DRAW_NOSHADE, 0.25f);
	p3dc_draw_model(&(camFront->vp), &(camFront->view_xfrm), P3DC_DRAW_NOSHADE, m);

	p3dc_draw_axes(&(camRight->vp), &(camRight->view_xfrm), P3DC_DRAW_NOSHADE, 0.25f);
	p3dc_draw_model(&(camRight->vp), &(camRight->view_xfrm), P3DC_DRAW_NOSHADE, m);

	p3dc_draw_axes(&(camOrtho->vp), &(camOrtho->view_xfrm), P3DC_DRAW_NOSHADE, 0.25f);
	p3dc_draw_model(&(camOrtho->vp), &(camOrtho->view_xfrm), P3DC_DRAW_NOSHADE, m);

	p3dc_flip_frame(1);
	p3dc_snapshot("test10.png", "Chuck McManis", NULL);
	p3dc_close();
	exit(0);
}
