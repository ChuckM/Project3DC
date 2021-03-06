/*
 * Planet.c
 *
 * This program tesselates a sphere using a lat/long type
 * patches.
 */

#include <stdio.h>
#include <math.h>

#define SEGMENTS 20

int _segments = SEGMENTS;

#define D2R(a)	((double)(a)/(double)(_segments)*2*3.14159)

double
getZ(int i) {
	double r = cos(D2R(i));
	if (fabs(r) < .00001) return 0.0;
	return r;
}

double
getY(int i, int j) {
	double r, y;
	r = sin(D2R(i));
	if (fabs(r) < .00001) r = 0;
	y = r * sin(D2R(j));
	if (fabs(y) < .00001) y = 0;
	return y;
}

double
getX(int i, int j) {
	double r, x;
	r = sin(D2R(i));
	if (fabs(r) < .00001) r = 0;
	x = r * cos(D2R(j));
	if (fabs(x) < .00001) x = 0;
	return x;
}

double
getTX(int j) {
	double tx; 

	tx = (double)j/(double)_segments * 255.0 + 0.5;
	if (fabs(tx) < 0.0001) tx = 0;
	return tx;
}

void
pvrt(i, j) {
	double tx, ty;
	int wrap;

	if (j == -1) {
		wrap = 1;
		j = 0;
	} else {
		wrap = 0;
	}
	tx = getTX(j+ wrap*_segments);
	ty = 128 - (((double)(i*2)/(double)_segments) * 127.5 + 0.5);
	if (fabs(ty) < 0.0001) ty = 0;
	printf("v%dr%d [%g, %g, %g] <%g, %g>", i, j,
		getX(i, j), getY(i, j), getZ(i), tx, ty);
}

void
main(int argc, char *argv[]) {
	int	i, j, vn = 0;
	double	r, x, y, z;

	if (argc == 2)
		_segments = atoi(argv[1]);
	
	printf("#\n");
	printf("# Autogenerated Sphere - %d segments\n", _segments);
	printf("#\n# This model was autogenerated by the planet.c program\n");
	printf("# in the util directory of the FreeBSD source tree.\n");
	printf("#\n");
	printf("model planet {\n");
	printf("	color base_color = 0, 0, 255 ;\n");
	printf("	texture base_texture = decal ;\n");
	printf("    vertex v0r0 = 0, 0, 1.0;\n");
	printf("	vertex v%dr0 = 0, 0, -1.0;\n", _segments);
	for (i = 1; i < (_segments/2); i++) {
		int sv = vn;
		z = cos(D2R(i));
		if (fabs(z) < .00001) z = 0;
		r = sin(D2R(i));
		if (fabs(r) < .00001) r = 0;
		for (j = 0; j < _segments; j++) {
			x = r * cos(D2R(j));
			if (fabs(x) < .00001) x = 0;
			y = r * sin(D2R(j));
			if (fabs(y) < .00001) y = 0;
			printf("    vertex v%dr%d = %g, %g, %g ;\n", i, j, x, y, z);
		}
		for (j = 0; j < _segments-1; j++) {
			if (i == 1) {
				printf("	face <base_texture> (base_color) ");
				pvrt(i, j+1); printf(",\n\t\t"); pvrt(i, j); printf(",\n\t\t");
				printf("v0r0 [0, 0, 1] <%g, 127.5>;\n", getTX(j)); 
			} else {
				printf("	face <base_texture> (base_color) ");
				pvrt(i, j+1); printf(",\n\t\t ");
				pvrt(i, j); printf(",\n\t\t ");
				pvrt(i-1, j); printf(",\n\t\t ");
				pvrt(i-1, j+1); printf(";\n");
			}
		}
		if (i == 1) {
			printf("	face <base_texture> (base_color) ");
			pvrt(i, -1); printf(",\n\t\t "); 
			pvrt(i, _segments-1); printf(",\n\t\t ");
			printf("v0r0 [0, 0, 1] <255.5, 127.5>;\n");
		} else {
			printf("	face <base_texture> (base_color) ");
			pvrt(i, -1); printf(",\n\t\t ");
			pvrt(i, _segments-1); printf(",\n\t\t ");
			pvrt(i-1, _segments-1); printf(",\n\t\t ");
			pvrt(i-1, -1); printf(";\n");
		}
	}
	i = (_segments/2) - 1;
	for (j = 0; j < _segments-1; j++) {
		printf(
		"	face <base_texture> (base_color) v%dr0 [0, 0, -1] <%g, 0.5>, ", 
				_segments, getTX(j));
		pvrt(i, j); printf(",\n\t\t "); 
		pvrt(i, j+1); printf(";\n");
	}
	printf(
		"	face <base_texture> (base_color) v%dr0 [0, 0, -1] <255.5, 0.5>, ", 
			_segments);
	pvrt(i, _segments-1); printf(",\n\t\t ");
	pvrt(i, -1); printf(";\n");
	printf("}\n");
}

