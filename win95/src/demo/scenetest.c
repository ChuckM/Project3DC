/*
 * Scene Test
 *
 * This program demonstrates some of the more interesting features of the
 * P3DC system. Further it provides an interesting benchmark for me to 
 * see how well the rendering speed is coming.
 *
 * $Id:$
 */
#include <stdio.h>
#include <windows.h>
#include <stdlib.h>
#include <math.h>
#include <p3dc.h>

extern int next_char();

void
main(int argc, char *argv[]) {
	p3dc_MODEL *airplane;
	p3dc_LIST *models = p3dc_new_list(P3DC_MODEL, P3DC_LIST_LINKED);
	p3dc_CMRA *main = p3dc_new_camera(36.5, 10.0, 10.0, -25.0);
	p3dc_CMRA *plane = p3dc_new_camera(45.0, -10.0, 5.0, -20.0);
	p3dc_CMRA *user = p3dc_new_camera(35.0, 0, 0, 0);
	p3dc_MODEL *m;
	p3dc_MODEL *cam_model, *plane_cam;
	p3dc_MODEL *earth, *moon;
	p3dc_VRTX *c_pos = p3dc_new_vrtx(0, 1.0, -3.0);
	p3dc_PNT3 u_pos;
	int wire_frame = 0;
	p3dc_NODE *t;
	int snap_shot_trigger = 0;
	float airplane_angle = 0;
	float camera_pitch, camera_yaw;
	float angle_inc;
	int frame_counter;

	p3dc_open();

	/* Main is a 16:9 "cinematic" view */
	p3dc_set_camera_frame(main, 0, 240, 320, 480);
	/* Plane is a square cockpit view */
	p3dc_set_camera_frame(plane, 0, 0, 320, 240);

	/* User camera is a standard 4:3 ration "screen" */
	p3dc_set_camera_frame(user, 320, 240, 640, 480);
	u_pos.x = -5.0;
	u_pos.y = 5.0;
	u_pos.z = -15.0;
	camera_pitch = 0;
	camera_yaw = 0;

	m = p3dc_new_model("checkerbox", 15.0, 15.0, 15.0);
	if (m) {
		p3dc_locate_model(m, -7.5, 0.0, -7.5);
		p3dc_add_node(models, p3dc_new_node(m, NULL, 0), P3DC_LIST_HEAD);
	}
	airplane = p3dc_new_model("airplane", 0.25, 0.25, 0.25);
	if (airplane == NULL) {
		p3dc_log("Couldn't load airplane, exiting.\n");
		p3dc_close();
		exit(0);
	}
	p3dc_locate_model(airplane, 4.0, 4.0, 0.0);
	p3dc_add_node(models, p3dc_new_node(airplane, NULL, 0), P3DC_LIST_HEAD);
	m = p3dc_new_model("cone", 1.0, 1.0, 10.0);
	if (m) {
		p3dc_pitch_model(m, 90.0f);
		p3dc_locate_model(m, 5.0, 5.0, 5.0);
		p3dc_model_set_all_colors(m, &COLOR_red);
		p3dc_add_node(models, p3dc_new_node(m, NULL, 0), P3DC_LIST_HEAD);
	}
	m = p3dc_new_model("cone", 1.0, 1.0, 5.5);
	if (m) {
		p3dc_pitch_model(m, 90.0f);
		p3dc_locate_model(m, -3.0, 2.5, 4.0);
		p3dc_model_set_all_colors(m, &COLOR_green);
		p3dc_add_node(models, p3dc_new_node(m, NULL, 0), P3DC_LIST_HEAD);
	}
	m = p3dc_new_model("cone", 3.0, 3.0, 5.5);
	if (m) {
		p3dc_pitch_model(m, 90.0f);
		p3dc_locate_model(m, 3.0, 2.5, 4.0);
		p3dc_model_set_all_colors(m, &COLOR_yellow);
		p3dc_add_node(models, p3dc_new_node(m, NULL, 0), P3DC_LIST_HEAD);
	}
	m = p3dc_new_model("cone", 2.0, 2.0, 5.5);
	if (m) {
		p3dc_pitch_model(m, 90.0f);
		p3dc_locate_model(m, 3.0, 2.5, -4.0);
		p3dc_model_set_all_colors(m, &COLOR_orange);
		p3dc_add_node(models, p3dc_new_node(m, NULL, 0), P3DC_LIST_HEAD);
	}
	m = p3dc_new_model("planet", 3.0, 3.0, 3.0);
	if (m) {
		earth = m;
		p3dc_model_set_color(m, "base_color", &COLOR_white);
		if (! p3dc_model_set_texture(m, "base_texture", "earth.3df"))
			p3dc_log("SUCCESS setting the model's texture.\n"); 
		p3dc_locate_model(m, 8.0, 4.0, -3.0);
		p3dc_pitch_model(m, -90);
		p3dc_add_node(models, p3dc_new_node(m, NULL, 0), P3DC_LIST_HEAD);
		moon = p3dc_new_model("planet", 1.5, 1.5, 1.5);
		if (moon) {
			p3dc_model_set_color(moon, "base_color", &COLOR_white);
			p3dc_model_set_texture(moon, "base_texture", "moon.3df");
			p3dc_locate_model(moon, 0.0, 8.0, -3.0);
			p3dc_pitch_model(moon, -90);
			p3dc_add_node(models, p3dc_new_node(moon, NULL, 0), P3DC_LIST_HEAD);
		}
	} else 
		earth = NULL;
	p3dc_log("reading camera.\n");
	cam_model = p3dc_new_model("camera", 0.25, 0.25, 0.25);
	if (cam_model) {
		p3dc_locate_model(cam_model, -5.0, 5.0, -5.0);
		p3dc_add_node(models, p3dc_new_node(cam_model, NULL, 0), P3DC_LIST_HEAD);
		p3dc_log("camera loaded.\n");
	} else {
		p3dc_log("Camera model didn't load.\n");
	}
	plane_cam = p3dc_new_model("camera", 0.15f, 0.15f, 0.15f);
	if (plane_cam) {
		p3dc_locate_model(plane_cam, -5.0, 5.0, -5.0);
		p3dc_add_node(models, p3dc_new_node(plane_cam, NULL, 0), P3DC_LIST_HEAD);
		p3dc_log("plane camera loaded.\n");
	} else {
		p3dc_log("Camera model didn't load.\n");
	}

	angle_inc = 0.25;
	frame_counter = 0;
	while (1) {
		float ax, ay, az;
		int	shifted;
		int first_model;
		p3dc_PNT3 pZ, up, out, loc;
		
		shifted = 0;

		if (GetAsyncKeyState(VK_CONTROL))
			shifted++;

		if (GetAsyncKeyState(VK_UP)) {
			if (shifted) {
				u_pos.z += 0.1f;
			} else {
				u_pos.y += 0.1f;
			}
		}
		if (GetAsyncKeyState(VK_DOWN)) {
			if (shifted) {
				u_pos.z -= 0.1f;
			} else {
				u_pos.y -= 0.1f;
			}
		}
		if (GetAsyncKeyState('U')) {
			camera_pitch += 0.5;
		}

		if (GetAsyncKeyState('D')) {
			camera_pitch -= 0.5;
		}
		if (GetAsyncKeyState('L')) {
			camera_yaw -= 0.5;
		}
		if (GetAsyncKeyState('R')) {
			camera_yaw += 0.5;
		}
		if (GetAsyncKeyState('W')) {
			wire_frame = P3DC_DRAW_WIRE;
		}
		if (GetAsyncKeyState('S')) {
			wire_frame = 0;
		}

		if (GetAsyncKeyState(VK_LEFT)) {
			u_pos.x -= 0.1f;
		}
		if (GetAsyncKeyState(VK_RIGHT)) {
			u_pos.x += 0.1f;
		}
		if (GetAsyncKeyState(VK_ESCAPE)) {
			return;
		}
		if (GetAsyncKeyState('F')) {
//			p3dc_set_camera_near(user, user->near_p + 0.1f);
		}
		if (GetAsyncKeyState('C')) {
//			p3dc_set_camera_near(user, user->near_p - 0.1f);
		}
		if (GetAsyncKeyState(VK_F1)) {
			snap_shot_trigger++;
		}

		ax = 25.0f * (float) sin(D2R(airplane_angle));
		az = 25.0f * (float) cos(D2R(airplane_angle));
		ay = 5.0f;
		/*
		 * Put the camera "model" in the scene where the user camera is
		 */
		pZ.x = airplane->O.data[0][3];
		pZ.y = airplane->O.data[1][3];
		pZ.z = airplane->O.data[2][3];
		p3dc_locate_model(cam_model, u_pos.x, u_pos.y, u_pos.z);
		p3dc_target_model(cam_model, airplane->O.data[0][3], 
									airplane->O.data[1][3],
									airplane->O.data[2][3]);
		p3dc_set_camera_origin(user, u_pos.x, u_pos.y, u_pos.z);
		p3dc_set_camera(user, P3DC_CAMERA_LOOK, &pZ, 0);

		if (earth)
			p3dc_roll_model(earth, .75f);

		p3dc_prepare_frame(&COLOR_black);
		p3dc_clear_viewport(&COLOR_white, &(main->vp));
		p3dc_clear_viewport(&COLOR_white, &(plane->vp));
		p3dc_clear_viewport(&COLOR_white, &(user->vp));
		first_model = 1;
		for (t = FIRST_NODE(models); t != NULL; t = NEXT_NODE(models)) {
			if (t->nxt == NULL)
				p3dc_display_ctrl(P3DC_CTRL_TRANSPARENT, 200);
			p3dc_draw_model(&(main->vp), &(main->view_xfrm), wire_frame, t->data.p);
			p3dc_draw_model(&(plane->vp), &(plane->view_xfrm), wire_frame, t->data.p);
			p3dc_draw_model(&(user->vp), &(user->view_xfrm), wire_frame, t->data.p);
			if (t->nxt == NULL) {
				p3dc_display_ctrl(P3DC_CTRL_OPAQUE, 0);
				first_model = 0;
			}
		}
		ax = 6.0f * (float) sin(D2R(airplane_angle));
		az = 6.0f * (float) cos(D2R(airplane_angle));
		ay = 2.0f;
		/*
		 * Compute airplane camera's new location as the airplane's
		 * origin plus the offset in DIST. Then extract the plane's
		 * orientation from from its "O" matrix and use that to 
		 * set the orientation and location of the camera that is
		 * tailing the plane.
		 */
		pZ.x = airplane->O.data[0][2];
		pZ.y = airplane->O.data[1][2];
		pZ.z = airplane->O.data[2][2];
		up.x = airplane->O.data[0][1];
		up.y = airplane->O.data[1][1];
		up.z = airplane->O.data[2][1];
		p3dc_xfrm_vrtx(NULL, &(airplane->O), c_pos);
		loc.x = c_pos->tv.x;
		loc.y = c_pos->tv.y;
		loc.z = c_pos->tv.z;
		/* Now set everything in one fell swoop! */
		p3dc_set_camera(plane, P3DC_CAMERA_ORIGIN, &loc, 
			P3DC_CAMERA_DIR, &pZ, P3DC_CAMERA_UP, &up,
			P3DC_CAMERA_PITCH, &camera_pitch, 
			P3DC_CAMERA_YAW, &camera_yaw, 0);

		if (plane_cam) {
			p3dc_locate_model(plane_cam, c_pos->tv.x, c_pos->tv.y, c_pos->tv.z);
			p3dc_point_model(plane_cam, &pZ, &up, 0);
			p3dc_pitch_model(plane_cam, camera_pitch);
			p3dc_yaw_model(plane_cam, camera_yaw);
		}

		ax = 6.0f * (float) sin(D2R(airplane_angle));
		az = 6.0f * (float) cos(D2R(airplane_angle));
		ay = 2.0f;
		out.x = ax;
		out.y = 0;
		out.z = az;
		up.x = 0;
		up.y = 1.0;
		up.z = 0;
		p3dc_cross_pnt3(&up, &out, &out);
		p3dc_fwd_model(airplane, 0.15f);
		p3dc_pitch_model(airplane, -0.75f);
		airplane_angle += angle_inc;
		if (airplane_angle > 360.0)
			airplane_angle = 360.0f - airplane_angle;

#if 0
		frame_counter++;
		if ((frame_counter % 30) == 0) {
			p3dc_log("Frame %d memory stats are :\n", frame_counter);
			p3dc_memstats(1);
		}
#endif
		p3dc_flip_frame(1);
		if (snap_shot_trigger) {
			p3dc_snapshot(NULL, "Chuck McManis", NULL);
			snap_shot_trigger = 0;
		}
	}

}
