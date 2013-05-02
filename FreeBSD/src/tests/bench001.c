/*
 * bench001 - bench mark #1
 *
 * This is the first in a series of modules used to benchmark/profile
 * various parts of P3DC. This first part looks at vertex transformations
 * which are, arguably, the most commonly called function.
 *
 * There are two transformations stored in a camera:
 *		world_xfrm - which transforms world space to camera space
 *		view_xfrm - which transforms world space to perspective view space
 */

#include <sys/types.h>
#include <stdio.h>
#include <sys/time.h>
#include <p3dc.h>

double
test(char *name, p3dc_XFRM *xfrm, int clear_xid) {
	struct timeval tv1, tv2;
	p3dc_VRTX *vrt = p3dc_new_vrtx(-10.f, -10.0f, -10.0f);
	int i;
	double res;
	u_long millis;

	if (clear_xid) {
		gettimeofday(&tv1, NULL);
		for (i = 0; i < 100000; i++) {
			p3dc_xfrm_vrtx(NULL, xfrm, vrt);
		}
		gettimeofday(&tv2, NULL);
	} else {
		gettimeofday(&tv1, NULL);
		for (i = 0; i < 100000; i++) {
			vrt->xid = 0;
			p3dc_xfrm_vrtx(NULL, xfrm,  vrt);
		}
		gettimeofday(&tv2, NULL);
	}
	millis = (tv2.tv_sec - tv1.tv_sec) * 1000000;
	millis += (tv2.tv_usec - tv1.tv_usec);
	res = (double)(millis) / 100000.0;
	printf("Execution %s was %ld uS or %g uS per %s transform.\n", 
		clear_xid ? "(accellerated) time" : "time", millis, res, name);
	p3dc_free_type(vrt);
	return res;
}

void
main(int argc, char *argv[]) {
	p3dc_CMRA *cmra = p3dc_new_camera(36.5, 10.0f, 10.0f, 10.0f);

	test("VIEW", &(cmra->view_xfrm), 0);
	test("VIEW", &(cmra->view_xfrm), 1);
	test("WORLD", &(cmra->world_xfrm), 0);
	test("WORLD", &(cmra->world_xfrm), 1);
}
