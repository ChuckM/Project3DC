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
	p3dc_CMRA *camera = p3dc_new_camera(90, 10, 10, -20);
	int flags = P3DC_DRAW_NOSHADE | P3DC_DRAW_WIRE;
	int done = 0;
	p3dc_FLOAT d, h, f;

	p3dc_open();
	scene = generate_scene();
	if (scene == NULL) {
		p3dc_log("Unable to generate scene, exiting.\n");
		exit(1);
	}
	d = 1.0f;
	h = 90.0f;
	f = 2048.0f;
	p3dc_dump_xfrm(&(camera->V2S), "Original Camera V2S\n", NULL, NULL, NULL);
	while (! done) {
		int c, process = 0;

		/* Configure the view to screen mapping. */
		p3dc_identity_xfrm(&P1); p3dc_identity_xfrm(&P2);
		if ((h > 178) || (h < 1.0)) h = 90.0f;
//		if ((d < 0)) d = 0;
		P1.data[0][0] = 1.0f / (float) tan(D2R(h/2.0));
		P1.data[1][1] = P1.data[0][0];
		P2.data[0][0] = camera->vp.scale_y / camera->vp.scale_x;
		P2.data[2][2] = 1.0f /(f-d);
		P2.data[3][2] = 1.0f;
		P2.data[2][3] = -d/(f-d);
		P2.data[3][3] = 0;
		p3dc_mult_xfrm(&P1, &P1, &P2);
		p3dc_set_camera_screen(camera, &P1);
		p3dc_dump_xfrm(&P1, "Computed Camera V2S\n", NULL, NULL, NULL);
		p3dc_prepare_frame(&COLOR_black);
		p3dc_draw_list(&(camera->vp), &(camera->view_xfrm), flags, scene);
		p3dc_draw_axes(&(camera->vp), &(camera->view_xfrm), flags, 0.25);
		p3dc_flip_frame(1);
		while (! process) {
			c = next_char();
			switch (c) {
			case 'q':
				done++;
				process++;
				break;
			case 'D':
				d += 0.1f;
				process++;
				break;
			case 'd':
				d -= 0.1f;
				process++;
				break;
			case 'H':
				h += 1.0f;
				process++;
				break;
			case 'h':
				h -= 1.0f;
				process++;
				break;
			case 'F':
				f += 1.0f;
				process++;
				break;
			case 'f':
				f -= 1.0f;
				process++;
				break;
			default:
				break;
			}
		}
	}
	p3dc_close();
	exit(0);
}