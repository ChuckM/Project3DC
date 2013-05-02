/*
 * CameraTest.c - Test out cameras and demonstrate what the various
 * parameters do to a camera.
 */

#include <stdio.h>
#include <p3dc.h>

extern int next_char();

void
main(int argc, char *argv[]) {
	p3dc_CMRA *main = p3dc_new_camera(45, 10, 10, -10);
	p3dc_CMRA *user = p3dc_new_camera(45, 40, 40, -25);
	p3dc_MODEL *earth;
	p3dc_MODEL *ref;
	int	c;
	float fov = 45;

	p3dc_open();
	ref = p3dc_new_model("checkerbox", 50.0, 1.0, 50.0);
	p3dc_locate_model(ref, -25, 0, -25);
	earth = p3dc_new_model("planet", 3.0, 3.0, 3.0); 
	p3dc_model_set_color(earth, "base_color", &COLOR_white);
	if (! p3dc_model_set_texture(earth, "base_texture", "earth.3df"))
			p3dc_log("SUCCESS setting the model's texture.\n"); 
	p3dc_locate_model(earth, 0, 4.0, 0.0);
	p3dc_pitch_model(earth, -90);
	p3dc_frame_camera(main, 0, 0, 320, 480);
	p3dc_frame_camera(user, 320, 0, 640, 480);
	while (1) {
		p3dc_prepare_frame(&COLOR_black);
		p3dc_draw_model(&(main->vp), &(main->view_xfrm), 0, earth);
		p3dc_draw_model(&(main->vp), &(main->view_xfrm), 0, ref);
		p3dc_draw_model(&(user->vp), &(user->view_xfrm), 0, earth);
		p3dc_draw_model(&(user->vp), &(user->view_xfrm), 0, ref);
		c = next_char();
		if (c == 'q') break;
		switch (c) {
		case '=':
			fov += 0.25;
			p3dc_fov_camera(user, fov);
			break;
		case '-':
			fov -= 0.25;
			p3dc_fov_camera(user, fov);
			break;
		}
		p3dc_flip_frame(1);
	}
	p3dc_close();

}