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

struct {
	char	*name;
	p3dc_CLR	*color;
	p3dc_FLOAT	x, y, z;
} items[] = {
	{"planet", &COLOR_blue, -2.0, 0.5, 0.0 },
	{"cube", &COLOR_green, 2.0, 0.5, 0.0 },
	{"cone", &COLOR_red, 0.0, 0.5, -2.0 },
	{"pyramid", &COLOR_yellow, 0.0, 0.5, 2.0 },
	{NULL, NULL, 0, 0, 0},
};

/*
 * Allocate and place a list of items into a scene.
 */
p3dc_LIST *
generate_scene() {
	p3dc_MODEL *m;
	p3dc_LIST *l;
	int	i;

	l = p3dc_new_list(P3DC_MODEL, P3DC_LIST_LINKED);
	if (l == NULL)
		return NULL;

	i = 0;
	while (1) {
		if (items[i].name == NULL)
			break;
		m = p3dc_new_model(items[i].name, 1.0, 1.0, 1.0);
		if (m != NULL) {
			p3dc_locate_model(m, items[i].x, items[i].y, items[i].z);
			p3dc_model_set_all_colors(m, items[i].color);
			p3dc_add_node(l, p3dc_new_node(m, NULL, 0), P3DC_LIST_HEAD);
		}
		i++;
	}
	return l;
}

extern int __p3dc_get_xform_id();
/*
 * Main program.
 *
 * Construct a scene, view it, and wait to exit.
 */
void
main(int argc, char *argv[]) {
	p3dc_VIEW	vp;
	p3dc_XFRM	view;
	p3dc_LIST	*models;
	p3dc_FLOAT	theta, phi, r;


	/*
	 * In this application we are manually creating the
	 * viewport and the view rather than using the camera
	 * system so that simpler viewing systems can be
	 * implemented and played with.
	 */

	theta = 45.0;
	phi = 45.0;
	r = 5.0;

	models = generate_scene();
	p3dc_open();
	if (models == NULL)
		exit(0);

	vp.scale_x = 1.0f/240.0f;
	vp.scale_y = 1.0f/240.0f;
	vp.center_x = 320.0f;
	vp.center_y = 240.0f;

	while(1) {
		p3dc_NODE *n;
		int	c;

		memset((char *)&view, 0, sizeof(view));
		view.data[0][0] = (float)(-sin(D2R(theta)));
		view.data[1][0] = (float)(-cos(D2R(theta))*cos(D2R(phi)));
		view.data[2][0] = (float)(-cos(D2R(theta))*sin(D2R(phi)));
		view.data[0][1] = (float)(cos(D2R(theta)));
		view.data[1][1] = (float)(-sin(D2R(theta))*cos(D2R(phi)));
		view.data[2][1] = (float)(-sin(D2R(theta))*sin(D2R(phi)));
		view.data[1][2] = (float)(sin(D2R(theta)));
		view.data[2][2] = (float)(-cos(D2R(theta)));
		view.data[2][3] = r;
		view.data[3][3] = 1.0;
		view.id = __p3dc_get_xform_id();

		p3dc_prepare_frame(&COLOR_black);
		for (n = FIRST_NODE(models); n != NULL; n = NEXT_NODE(models)) {
			p3dc_MODEL *m = (p3dc_MODEL *)(n->data.p);

			p3dc_draw_model(&vp, &view, P3DC_DRAW_NOSHADE, m);
		}
		p3dc_flip_frame(1);
		while (1) {
			c = next_char();
			switch (c) {
			case 'q':
				p3dc_close();
				exit(0);
			default:
				break;
			}
		}
	}
}
