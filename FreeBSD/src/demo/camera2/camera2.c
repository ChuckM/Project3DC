/*
 * perspective.c - This module is used to explore the parameters
 * of the perspective transform in the camera.
 *
 * $Id:$
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <p3dc.h>

extern p3dc_LIST *generate_scene(void);
extern int next_char();
extern void p3dc_dump_xfrm(p3dc_XFRM *, char *, char *, char *, char *);

void
main(int argc, char *argv[]) {
	p3dc_LIST *scene;
	p3dc_XFRM P1, P2;
	p3dc_CMRA *camera = p3dc_new_camera(15, -5, 0, 0);
	int flags = P3DC_DRAW_NOSHADE | P3DC_DRAW_WIRE;
	int done = 0;
	p3dc_FLOAT d, h, f;

	d = 1.0f;
	h = 0.5f;
	f = 256.0f;
	if (argc > 1) {
		argc-- ; argv++;
		d = atof(*argv);
	}

	if (argc > 1) {
		argc-- ; argv++;
		h = atof(*argv);
	}

	if (argc > 1) {
		argc-- ; argv++;
		f = atof(*argv);
	}

	printf("Perspective parameters: d=%g, h=%g, f=%g\n", d, h, f);
	p3dc_open();
	scene = generate_scene();
	if (scene == NULL) {
		p3dc_log("Unable to generate scene, exiting.\n");
		exit(1);
	}
	/* Configure the view to screen mapping. */
	p3dc_identity_xfrm(&P1); p3dc_identity_xfrm(&P2);
	if (EQZ(h) || (h < 0)) h = 1.0f;
	if (EQZ(d) || (d < 0)) d = 1.0f;
	P1.data[0][0] = d/h;
	P1.data[1][1] = P1.data[0][0];
	P2.data[0][0] = camera->vp.scale_y / camera->vp.scale_x;
	P2.data[2][2] = f/(f-d);
	P2.data[3][2] = 1.0f;
	P2.data[2][3] = -(f*d)/(f-d);
	P2.data[3][3] = 0;
	p3dc_mult_xfrm(&P1, &P1, &P2);
	p3dc_set_camera_screen(camera, &P1);
	p3dc_prepare_frame(&COLOR_black);
	p3dc_draw_list(&(camera->vp), &(camera->view_xfrm), flags, scene);
	p3dc_draw_axes(&(camera->vp), &(camera->view_xfrm), flags, 0.25);
	p3dc_flip_frame(1);
	p3dc_snapshot("camera2.png", NULL, NULL);
	p3dc_close();
	exit(0);
}
