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

extern int next_char();


extern int __p3dc_get_xform_id();
extern void p3dc_view_camera(p3dc_CMRA *, p3dc_XFRM *);
extern p3dc_LIST *generate_scene(void);

/*
 * Main program.
 *
 * Construct a scene, view it, and wait to exit.
 */
void
main(int argc, char *argv[]) {
	p3dc_XFRM	view;
	p3dc_LIST	*models;
	p3dc_CMRA	*cam = p3dc_new_camera(45.0f, 5.0, 5.0, 5.0);
	p3dc_FLOAT	theta, phi, r;
	int shade = P3DC_DRAW_NOSHADE, wire = P3DC_DRAW_WIRE;


	/*
	 * In this application we are manually creating the
	 * viewport and the view rather than using the camera
	 * system so that simpler viewing systems can be
	 * implemented and played with.
	 */

	theta = -45.0f;
	phi = 54.74f;
	r = 15.0f;

	models = generate_scene();
	p3dc_open();
	if (models == NULL)
		exit(0);

	while(1) {
		int	c;

		/* 
	     * Start out with a 1:1 transformation:
		 *	-- the world and camera at 0,0,0 
		 */
		p3dc_identity_xfrm(&view);

		/* 
		 * Now add a rotation of the world about the 
		 * about the Y (up) axis.
		 */
		p3dc_yrot_xfrm(&view, phi);

		/*
		 * Now add a rotation of the world about the X
		 * or "left" axis.
		 */
		p3dc_xrot_xfrm(&view, theta);

		/*
		 * And finally move out some number of units from
		 * the origin.
		 */
		p3dc_locate_xfrm(&view, 0, 0, r);

		/*
		 * Combine it with the standard perspective/screen transform
		 */
		p3dc_set_camera_view(cam, &view);

		/*
		 * And then render the scene with this camera.
		 */
		p3dc_prepare_frame(&COLOR_black);
		p3dc_draw_list(&(cam->vp), &(cam->view_xfrm), wire | shade, models);
		p3dc_draw_axes(&(cam->vp), &(cam->view_xfrm), P3DC_DRAW_NOSHADE, 0.25f);

		/* Finally, display this frame on the screen */
		p3dc_flip_frame(1);

		/* Wait for the user to change something */
		while (1) {
			int cont = 0;
			c = next_char();
			switch (c) {
			case 'w':
				wire = (wire) ? 0 : P3DC_DRAW_WIRE;
				cont++;
				break;
			case 's':
				shade = (shade) ? 0 : P3DC_DRAW_NOSHADE;
				cont++;
				break;
			case 'q':
				p3dc_close();
				exit(0);
			case 'T':
				theta += 1.0;
				cont++;
				break;
			case '*':
				p3dc_snapshot(NULL, NULL, NULL);
				break;
			case 't':
				theta -= 1.0;
				cont++;
				break;
			case 'P':
				phi += 1.0;
				cont++;
				break;
			case 'p':
				phi -= 1.0;
				cont++;
				break;
			case 'R':
				r += 1.0;
				cont++;
				break;
			case 'r':
				r -= 1.0;
				cont++;
				break;
			default:
				break;
			}
			if (cont) break;
		}
	}
}
