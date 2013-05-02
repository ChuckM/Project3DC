/*
 * A Simple model test program it instantiates a model
 * then draws it and moves it about.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <p3dc.h>

extern void p3dc_locate_model(p3dc_MODEL *, p3dc_FLOAT, p3dc_FLOAT, p3dc_FLOAT);
extern void p3dc_point_model(p3dc_MODEL *, p3dc_FLOAT, p3dc_FLOAT, p3dc_FLOAT);

p3dc_LIST lines = {NULL, NULL, P3DC_LINE, NULL };
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
						p3dc_new_node(ll, NULL, P3DC_NODE_FREEPAYLOAD), 
						P3DC_LIST_TAIL);
				if ((fmod(i,10)) == 0)
					ccc = &index_color;
				else
					ccc = &line_color;
				b.x = i; b.z = -j;
				ll = p3dc_new_line(ccc, &a, &b);
				p3dc_add_node(&lines, 
						p3dc_new_node(ll, NULL, P3DC_NODE_FREEPAYLOAD),
						P3DC_LIST_TAIL);
			}
		}

	}
	p3dc_draw_list(&(cam->vp), &(cam->view_xfrm), P3DC_DRAW_NOSHADE, &lines);
}

extern int __p3dc_debug_trigger;

void
main(int argc, char *argv[]) {
	p3dc_MODEL *arrow;
	p3dc_CMRA *cam = p3dc_new_camera(35, 25, 25, -45);
	float cx, cz, rot;
	int snap_shot_trigger = 0;
	int c;

	p3dc_log("Reading model.\n");
	arrow = p3dc_new_model("airplane", 1.0, 1.0, .4);
	p3dc_log("Done reading result was %s\n", (arrow == NULL) ? "NULL" : "non-null");
	p3dc_log_flush();
	if (arrow == NULL) {
		p3dc_log_close();
		exit(1);
	}
	p3dc_dump_model(arrow);
	p3dc_open();
	rot = 0;
	cx = (float)sin(rot*DTR);
	cz = (float)cos(rot*DTR);
	if (EQZ(cx)) cx = 0;
	if (EQZ(cz)) cz = 0;
	p3dc_prepare_frame(&COLOR_white);
	p3dc_locate_model(arrow, 10, 10, -10);
	p3dc_draw_axes(&(cam->vp), &(cam->view_xfrm), P3DC_DRAW_NOSHADE, .75f);
	p3dc_draw_model(&(cam->vp), &(cam->view_xfrm), 0, arrow);
	draw_lines(cam);
	if (rot > 360) rot = 360 - rot;
	if (rot < 0 ) rot = 360 + rot;
	p3dc_flip_frame(1);
	p3dc_snapshot("modeltest.png", "Chuck McManis", NULL);
	p3dc_close();
	exit(0);
}
