/*
 * A Simple model test program it instantiates a model
 * then draws it and moves it about.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <p3dc.h>

extern int next_char();
extern void p3dc_locate_model(p3dc_MODEL *, p3dc_FLOAT, p3dc_FLOAT, p3dc_FLOAT);
extern void p3dc_point_model(p3dc_MODEL *, p3dc_FLOAT, p3dc_FLOAT, p3dc_FLOAT);

p3dc_LIST lines = {NULL, NULL, P3DC_LINE, P3DC_LIST_LINKED, NULL };
p3dc_CLR line_color = { 0xa0a0d0ff };
p3dc_CLR  index_color = { 0x6060d0ff } ;

void
draw_lines(p3dc_CMRA *cam) {
	if (lines.head == NULL) {
		p3dc_FLOAT i, j;
		for (i = -20; i < 21; i +=2) {
			for (j = -20; j < 21; j+= 2) {
				p3dc_LINE *ll;
				p3dc_CLR *ccc;
				p3dc_PNT3 a, b;

				a.y = b.y = -0.5;
				a.x = i; a.z = j;
				b.x = -i; b.z = j;
				if ((fmod(j,10)) == 0)
					ccc = &index_color;
				else
					ccc = &line_color;
				ll = p3dc_new_line(ccc, &a, &b);
				p3dc_add_node(&lines, 
					p3dc_new_node(ll, NULL, P3DC_NODE_FREEPAYLOAD), P3DC_LIST_TAIL);
				if ((fmod(i,10)) == 0)
					ccc = &index_color;
				else
					ccc = &line_color;
				b.x = i; b.z = -j;
				ll = p3dc_new_line(ccc, &a, &b);
				p3dc_add_node(&lines, 
						p3dc_new_node(ll, NULL, P3DC_NODE_FREEPAYLOAD), P3DC_LIST_TAIL);
			}
		}

	}
	p3dc_draw_list(&(cam->vp), &(cam->view_xfrm), P3DC_DRAW_NOSHADE, &lines);
}

extern int __p3dc_debug_trigger;
extern int __p3dc_normal_view;
extern p3dc_FLOAT __p3dc_normal_size;

void
main(int argc, char *argv[]) {
	p3dc_MODEL *arrow;
	p3dc_CMRA *cam = p3dc_new_camera(35, 25, 25, -45);
	float cx, cz, rot;
	p3dc_CLR pink;
	int snap_shot_trigger = 0;
	int draw_flags;
	int c;

	pink.gRED = 0;
	pink.gGREEN = 255;
	pink.gBLUE = 255;
	pink.gALPHA = 255;
	draw_flags = 0;
	__p3dc_normal_view = 0;
	__p3dc_normal_size = 3.0f;
	p3dc_log("Reading model.\n");
	arrow = p3dc_new_model("planet", 5.5f, 5.5f, 5.5f);
	if (arrow == NULL) {
		p3dc_log_close();
		exit(1);
	}
	p3dc_open();
//	if (! p3dc_model_set_texture(arrow, "base_texture", "std_texture.3df"))
//		p3dc_log("SUCCESS setting the model's texture.\n"); 
		
	if (! p3dc_model_set_all_colors(arrow, &pink)) {
		p3dc_log("Changed color 'yellow' to pink.\n");
	}
	if (! p3dc_model_set_all_textures(arrow, "moon.3df")) {
		p3dc_model_set_all_colors(arrow, &COLOR_white);
		p3dc_log("SUCCESS setting the model's texture.\n"); 
	}
	p3dc_log("Done reading result was %s\n", (arrow == NULL) ? "NULL" : "non-null");
	p3dc_dump_model(arrow);
	p3dc_log_flush();
	rot = 0;
	while(1) {
		cx = (float)sin(rot*DTR);
		cz = (float)cos(rot*DTR);
		if (EQZ(cx)) cx = 0;
		if (EQZ(cz)) cz = 0;
		p3dc_prepare_frame(&COLOR_white);
		p3dc_draw_axes(&(cam->vp), &(cam->view_xfrm), P3DC_DRAW_NOSHADE, .75f);
		p3dc_draw_model(&(cam->vp), &(cam->view_xfrm), draw_flags, arrow);
		draw_lines(cam);
		if (rot > 360) rot = 360 - rot;
		if (rot < 0 ) rot = 360 + rot;
		p3dc_flip_frame(1);
		if (snap_shot_trigger) {
			p3dc_snapshot("test.png", "Chuck McManis", NULL);
			snap_shot_trigger = 0;
		}
#define MOVE_DELTA	1.5f
#define ROT_DELTA 2.0f
		switch(c = next_char()) {
		case '!':
			p3dc_log("Breakpoint target.\n");
			break;
		case 'b':
			draw_flags = (draw_flags == 0) ? P3DC_DRAW_BBOX : 0;
			break;
		case 'q':
		case 'Q':
			p3dc_close();
			exit(0);
		case 'y':
			p3dc_locate_model(arrow, arrow->loc.x, arrow->loc.y+MOVE_DELTA, arrow->loc.z);
			break;
		case 'n':
			p3dc_locate_model(arrow, arrow->loc.x, arrow->loc.y-MOVE_DELTA, arrow->loc.z);
			break;
		case 'j':
			p3dc_locate_model(arrow, arrow->loc.x+MOVE_DELTA, arrow->loc.y, arrow->loc.z);
			break;
		case 'g':
			p3dc_locate_model(arrow, arrow->loc.x-MOVE_DELTA, arrow->loc.y, arrow->loc.z);
			break;
		case 't':
			p3dc_roll_model(arrow, 1);
			break;
		case 'u':
			p3dc_roll_model(arrow, -1); 
			break;
		case 'r':
			p3dc_locate_model(arrow, arrow->loc.x, arrow->loc.y, arrow->loc.z+MOVE_DELTA);
			break;
		case 'v':
			p3dc_locate_model(arrow, arrow->loc.x, arrow->loc.y, arrow->loc.z-MOVE_DELTA);
			break;
		case 'z':
			p3dc_yaw_model(arrow, ROT_DELTA);
			break;
		case 'Z':
			p3dc_pitch_model(arrow, ROT_DELTA);
			break;
		case 'C':
			p3dc_pitch_model(arrow, -ROT_DELTA);
			break;
		case 'c':
			p3dc_yaw_model(arrow, -ROT_DELTA);
			break;
		case '1':
			__p3dc_debug_trigger = 1;
			break;
		case '0':
			__p3dc_debug_trigger = 0;
			break;
		case '*':
			snap_shot_trigger = 1;
			break;
		case 'N':
			__p3dc_normal_view = ! __p3dc_normal_view;
		}
	}
	p3dc_close();
}