#include <stdio.h>
#include <p3dc.h>

void
main(int argc, char *argv[]) {
	p3dc_FLOAT	deg = 75;
	p3dc_FLOAT  rad = PI/3;

	printf(" D2R(%g) = %g = %g\n", deg, D2R(deg), R2D(D2R(deg)));
	printf(" R2D(%g) = %g = %g\n", rad, R2D(rad), R2D(D2R(rad)));
}
