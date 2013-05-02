/*
 * Memory allocator test 001 - get_type/free_type verification
 */

#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <p3dc.h>

void
main(int argc, char *argv[]) {
	p3dc_VRTX 	*v[25];
	p3dc_CLR	*c[25];
	struct timeval tv1, tv2;
	int i, ndx;
	u_long millis;
	int			v_allocs = 0, v_frees = 0;
	int			c_allocs = 0, c_frees = 0;

	memset((char *)v, 0, sizeof(p3dc_VRTX *) * 25);
	memset((char *)c, 0, sizeof(p3dc_CLR *) * 25);
	p3dc_log_open("mem001.log");
	srand(0);
	gettimeofday(&tv1, 0);
	for (i = 0; i < 1000000; i++) {
		ndx = rand() % 25;
		if (v[ndx] == NULL) {
			v[ndx] = p3dc_new_vrtx(1.0f, 1.0f, 2.0f);
			v_allocs++;
		} else {
			p3dc_free_vrtx(&(v[ndx]));
			v_frees++;
			v[ndx] = NULL;
		}
		ndx = rand() % 25;
		if (c[ndx] == NULL) {
			c[ndx] = p3dc_new_type(P3DC_CLR);
			c_allocs++;
		} else {
			p3dc_free_type(c[ndx]);
			c[ndx] = NULL;
			c_frees++;
		}
	}
	gettimeofday(&tv2, 0);
	millis = (tv2.tv_sec - 1) - tv1.tv_sec;
	millis += ((tv2.tv_usec + 1000000) - tv1.tv_usec);
	printf("Time to complete %d/%d allocs and %d/%d frees was %ld uSecs\n",
			v_allocs, c_allocs, v_frees, c_frees, millis);
	p3dc_memstats(0);
	exit(0);
}

