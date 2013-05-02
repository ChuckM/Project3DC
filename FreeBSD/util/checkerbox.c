/*
 * Generate a checkerbox model which consists of
 * an array of vertices all in the same plane
 */

#include <stdio.h>

void
main(int argc, char *argv[]) {
	int 	i, j;
	float 	x, y, z;
	int num = 8;
	int dark_bit = 0;
	int vnum = 0;

	printf("model checkerbox {\n");
	printf("	origin 0, 0, 0;\n");
	printf("	color bright = 255, 255, 255 ;\n");
	printf("	color dark = 255, 0, 0 ;\n");
	y = 0.0f;
	for (i = 0; i < num; i++) {
		z = (float) i / (float) (num - 1);
		for (j = 0; j < num; j++) {
			x = (float) j / (float) (num - 1);
			printf("	Vertex v%d = %g, %g, %g ; \n", ++vnum, x, y, z);
		}
	}
	for (i = 0; i < (num * num) - num; i++) {
		if (((i+1) % num) == 0) {
			dark_bit = (dark_bit == 0) ? 1 : 0; 
			continue;
		}
		printf("	Face (%s) v%d, v%d, v%d, v%d ;\n",
			((i & 1) == dark_bit) ? "bright" : "dark",
			i+1, i+2, i+2+num, i+1+num);
	}
	printf("}\n");
}
			
