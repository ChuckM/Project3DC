/*
 * BasicScene.c - Create a really basic scene (four geometric shapes
 * around the origin and a set of axes.
 *
 * $Id:$
 */
#include <stdio.h>
#include <math.h>
#include <p3dc.h>

struct {
	char	*name;
	p3dc_CLR	*color;
	p3dc_FLOAT	x, y, z, zrot, scale;
} items[] = {
	{"planet", &COLOR_blue, -2.0, 1.0, 0.0, 90 , 1.0},
	{"cube", &COLOR_green, 2.0, 1.0, 0.0, 0, 2.0 },
	{"cone", &COLOR_red, 0.0, 1.0, -2.0, 90, 2.0 },
	{"pyramid", &COLOR_purple, 0.0, 1.0, 2.0, 0, 2.0 },
	{NULL, NULL, 0, 0, 0, 0},
};

/*
 * Allocate and place a list of items into a scene.
 */
p3dc_LIST *
generate_scene() {
	p3dc_MODEL *m;
	p3dc_LIST *l;
	p3dc_LINE *line;
	p3dc_PNT3	a, b;
	int	i;

	l = p3dc_new_list(P3DC_UNKNOWN, P3DC_LIST_LINKED);
	if (l == NULL)
		return NULL;

	for (i = -5; i < 6; i++) {
		a.x = (float) i; a.y = 0; a.z = -5.0f;
		b.x = (float) i; b.y = 0; b.z =  5.0f;
		line = p3dc_new_line(&COLOR_white, &a, &b);
		p3dc_add_node(l, p3dc_new_node(line, NULL, 0), P3DC_LIST_HEAD);
		a.x = -5.0f; a.y = 0; a.z = (float) i;
		b.x =  5.0f; b.y = 0; b.z = (float) i;
		line = p3dc_new_line(&COLOR_white, &a, &b);
		p3dc_add_node(l, p3dc_new_node(line, NULL, 0), P3DC_LIST_HEAD);
	}
	i = 0;
	while (1) {
		if (items[i].name == NULL)
			break;
		m = p3dc_new_model(items[i].name, items[i].scale, items[i].scale, items[i].scale);
		if (m == NULL) {
			p3dc_log("%s failed to load.\n", items[i].name);
			return (NULL);
		}
		if (m != NULL) {
			p3dc_locate_model(m, items[i].x, items[i].y, items[i].z);
			p3dc_model_set_all_colors(m, items[i].color);
			if (! EQZ(items[i].zrot))
				p3dc_pitch_model(m, items[i].zrot);
			p3dc_add_node(l, p3dc_new_node(m, NULL, 0), P3DC_LIST_HEAD);
		}
		i++;
	}
	return l;
}
