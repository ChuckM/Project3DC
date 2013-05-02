/*
 *  Make some field of view calculations.
 *  Film size 35mm negative
 *		36mm wide x 24mm tall.
 *
 * Useful optical formulas
 *	Specs & Measurements.
 */
#include <stdio.h>
#include <math.h>

#define PI			(3.14159265358979323846)
#define R2D(r)		((r)*180.0/PI)

void
main(int argc, char *argv[]) {
	double fov;
	double a;

	for (a = 18.0; a < 205; a+= 1.0) {
		printf("%gmm lens - f.o.v = %g degrees.\n", a, 2.0*R2D(atan2(18.0, a)));
	}
	exit(0);
}

