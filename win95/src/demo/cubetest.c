/*
 * CubeTest.c - A really basic functional test for p3dc
 *
 * This program should draw a cube using lines and then
 * display it.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "p3dc.h"

/*
 * Bounding box corners of the square are as follows:
 *
 *                        8*
 *                6 +------+ 7
 *                 /|     /|
 *                / |    / |
 *               /  |   /  |          +z  
 *              / 5 +--/---+ 4    +y  /
 *             /   /  /   /       |  /
 *          3 +------+ 2 /        | /
 *            |  /   |  /         |/
 *            | /    | /          +-----> +x
 *            |/     |/
 *          0 +------+ 1
 *
 */

p3dc_PNT3 dot[] = {
	{ -5.0, -5.0, -5.0 },
	{  5.0, -5.0, -5.0 },
	{  5.0,  5.0, -5.0 },
	{ -5.0,  5.0, -5.0 },
	{  5.0, -5.0,  5.0 },
	{ -5.0, -5.0,  5.0 },
	{ -5.0,  5.0,  5.0 },
	{  5.0,  5.0,  5.0 },
	{  0.0,  0.0,  20.0 },
};

p3dc_PNT2 cube_edges[] = {
	{ 0, 1},	// 0
	{ 1, 2},	// 1
	{ 2, 3},	// 2
	{ 3, 0},	// 3
	{ 3, 4},	// 4
	{ 2, 5},	// 5
	{ 4, 5},	// 6
	{ 5, 6},	// 7
	{ 6, 7},	// 8
	{ 7, 0},	// 9
	{ 6, 1},	// 10
	{ 7, 4},	// 11
	{ 8, 0},	// 12
	{ 8, 1},	// 13
	{ 8, 6},	// 14
	{ 8, 7},	// 15

};

p3dc_PNT3 views[10] = {
	{ 75, 75, -75 },
	{ 0, 75, -75},
	{ -75, 75, -75 },
	{ -75, 75, 0 },
	{ -75, 75, 75 },
	{ 0, 75, 75 },
	{ 75, 75, 75 },
	{ 75, 75, 0},
	{ 0, 75, 0 },
	{ 0, -75, 0},
};

extern int next_char();

static struct line_data {
	p3dc_CLR		*c;	/* Line color */
	p3dc_PNT3	*s;	/* Start point */
	p3dc_PNT3	*e;	/* End point */
} sqr_line[] = {
	{ &COLOR_green, &dot[0], &dot[1]},
	{ &COLOR_green, &dot[1], &dot[2]},
	{ &COLOR_green, &dot[2], &dot[3]},
	{ &COLOR_green, &dot[3], &dot[0]},

	{ &COLOR_blue, &dot[4], &dot[5]},
	{ &COLOR_blue, &dot[5], &dot[6]},
	{ &COLOR_blue, &dot[6], &dot[7]},
	{ &COLOR_blue, &dot[7], &dot[4]},

	{ &COLOR_yellow, &dot[0], &dot[5]},
	{ &COLOR_yellow, &dot[1], &dot[4]},
	{ &COLOR_yellow, &dot[2], &dot[7]},
	{ &COLOR_yellow, &dot[3], &dot[6]},

	{ &COLOR_red, &dot[4], &dot[8]},
	{ &COLOR_red, &dot[5], &dot[8]},
	{ &COLOR_red, &dot[6], &dot[8]},
	{ &COLOR_red, &dot[7], &dot[8]},
	{ NULL, NULL, NULL}
};

static struct point_data {
	p3dc_CLR		*c;	/* point color */
	p3dc_PNT3	*p;	/* point pos */
} sqr_point[] = {
	{ &COLOR_green, &dot[0]},
	{ &COLOR_green, &dot[1]},
	{ &COLOR_green, &dot[2]},
	{ &COLOR_green, &dot[3]},

	{ &COLOR_blue, &dot[4]},
	{ &COLOR_blue, &dot[5]},
	{ &COLOR_blue, &dot[6]},
	{ &COLOR_blue, &dot[7]},

	{ NULL, NULL }
};

static struct polygon_data {
	p3dc_CLR		*color;	/* Polygon color */
	p3dc_PNT3	*a, *b, *c, *d;	/* Polygon vertices */
} sqr_polygon[] = {
	{ &COLOR_green, &dot[0], &dot[1], &dot[2], &dot[3] },
	{ &COLOR_blue, &dot[4], &dot[5], &dot[6], &dot[7] },
	{ &COLOR_yellow, &dot[0], &dot[3], &dot[6], &dot[5] },
	{ &COLOR_yellow, &dot[3], &dot[2], &dot[7], &dot[6] },
	{ &COLOR_yellow, &dot[1], &dot[4], &dot[7], &dot[2] },
	{ &COLOR_yellow, &dot[0], &dot[5], &dot[4], &dot[1] },
	{ &COLOR_red, &dot[4], &dot[8], &dot[7], NULL },
	{ &COLOR_red, &dot[5], &dot[6], &dot[8], NULL },
	// { NULL, NULL, NULL, NULL },
	{ &COLOR_red, &dot[7], &dot[8], &dot[4], NULL },
	{ &COLOR_red, &dot[8], &dot[6], &dot[5], NULL },
	{ NULL, NULL, NULL, NULL }
};


static p3dc_LIST sqr1 = { NULL, NULL, P3DC_POINT };
static p3dc_LIST sqr2 = { NULL, NULL, P3DC_LINE };
static p3dc_LIST sqr3 = { NULL, NULL, P3DC_POLY };

 
p3dc_LIGHT red_lights = {
		{ 1.0f, 0.0f, 0.0f }, 
		{ 0.57735027f, -0.57735027f, 0.57735027f },
		{ 0, 0, 0},			
		{ 0, 0, 0},			
		0, P3DC_LIGHT_DIRECTIONAL, 
};

/*
 * Draw the major cartesian axes into the world.
 */
void
draw_thing(p3dc_CMRA *cam, int num) {
	int i = 0;
	p3dc_NODE *n;

	if (sqr2.head == NULL) {
		while(sqr_line[i].c) {
			p3dc_LINE *ll;
			struct line_data *ld = &(sqr_line[i]);

			ll = p3dc_new_line(ld->c, ld->s, ld->e);
			n = p3dc_new_node(ll, NULL, 0);
			p3dc_add_node(&sqr2, n, P3DC_LIST_TAIL);
			i++;
		}
	}
	i = 0;
	if (sqr1.head == NULL) {
		while(sqr_point[i].c) {
			p3dc_POINT *pp;
			struct point_data *pd = &(sqr_point[i]);
			pp = p3dc_new_point(pd->c, pd->p);
			n = p3dc_new_node(pp, NULL, 0);
			p3dc_add_node(&sqr1, n, P3DC_LIST_TAIL);
			i++;
		}
	}

	i = 0;
	if (sqr3.head ==  NULL) {
		p3dc_PNT3 vv[4];
		while(sqr_polygon[i].c) {
			p3dc_POLY *pp;
			int size = 4;
			struct polygon_data *pd = &(sqr_polygon[i]);
			vv[0] = *(pd->a);
			vv[1] = *(pd->b);
			vv[2] = *(pd->c);
			if (pd->d) {
				vv[3] = *(pd->d);
			} else
				size = 3;
			pp = p3dc_new_polygon(pd->color, size, vv); 
			n = p3dc_new_node(pp, NULL, 0);
			p3dc_add_node(&sqr3, n, P3DC_LIST_TAIL);
			i++;
		}
	}

	switch (num) {
	case 1:
		p3dc_draw_list(&(cam->vp), &(cam->view_xfrm), 0, &sqr2);
		break;
	case 2:
		p3dc_draw_list(&(cam->vp), &(cam->view_xfrm), 0, &sqr3);
		break;
	default:
		p3dc_draw_list(&(cam->vp), &(cam->view_xfrm), 0, &sqr1);
		break;
	}
	return;
}

void
main(int argc, char *argv[]) {
	p3dc_CMRA *cam = p3dc_new_camera(37, views[0].x, views[0].y, views[0].z);
	int draw_square = 1;
	int do_rotate = 0, do_roll = 0;
	float cx, cz, rot = 0;
	int i, item = 0;
	float factor = 1.0;
	int disco = 0;
	p3dc_PNT3 x1 = {-10, -10, 10};
	p3dc_PNT3 x2 = {-10.2f, -10.2f, 10.2f};
	p3dc_POINT *origin = p3dc_new_point(&COLOR_blue, &x1);
	p3dc_POINT *ctrl = p3dc_new_point(&COLOR_red, &x2);
	
	p3dc_open();
	while (1) {
		p3dc_prepare_frame(&COLOR_black);
		cx = (float)(75 * cos(rot * DTR)+cam->look.x);
		cz = (float)(75 * sin(rot * DTR)+cam->look.z);
		rot = rot + factor;
		if (rot > 360) rot = rot - 360;
		p3dc_draw_point(&(cam->vp), &(cam->view_xfrm), 0, origin);
		p3dc_draw_point(&(cam->vp), &(cam->view_xfrm), P3DC_DRAW_NOSHADE, ctrl);
		p3dc_draw_axes(&(cam->vp), &(cam->view_xfrm), P3DC_DRAW_NOSHADE, 1.0);
		draw_thing(cam, item);
		p3dc_flip_frame(1);
		switch (i = next_char()) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			p3dc_locate_camera(cam, views[i - '0'].x, 
									views[i - '0'].y, 
										views[i - '0'].z);
			break;
		case 't':
			item = (item+1) % 3;
			break;

		case 'r':
			do_rotate = ! do_rotate;
			break;

		case 'R':
			do_roll = ! do_roll;
			break;

		case 'q':
		case 'Q':
			p3dc_close();
			exit(0);
			break;
		case 'l':
			p3dc_target_camera(cam, cam->look.x+1.0f, 0.0, 0.0);
			break;
		case 'D':
			disco = ! disco;
			if (disco) {
				p3dc_light_add(&red_lights);
			} else {
				p3dc_light_init();
			}
			break;
		case '~':
			p3dc_roll_camera(cam, 0);
			break;
		case '!':
			p3dc_roll_camera(cam, 15);
			break;
		case '@':
			p3dc_roll_camera(cam, 30);
			break;
		case '#':
			p3dc_roll_camera(cam, 45);
			break;
		case '$':
			p3dc_roll_camera(cam, 60);
			break;
		case '%':
			p3dc_roll_camera(cam, 75);
			break;
		case '^':
			p3dc_roll_camera(cam, 90);
			break;
		case '=':
			factor++;
			break;
		case '-':
			factor--;
			break;

		}

		if (do_rotate)
			p3dc_locate_camera(cam, cx, 25, cz);
		if (do_roll)
			p3dc_roll_camera(cam, rot);
	}
	p3dc_close();
	exit(0);
}
