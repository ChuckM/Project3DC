#include <stdio.h>
#include <stdlib.h>
#include <p3dc.h>

void
dump_vrtx(char *s, p3dc_VRTX *v) {
  printf("%s : [%7.4f, %7.4f, %7.4f] => ",
		s, v->v.x, v->v.y, v->v.z);
  printf("[%7.4f, %7.4f, %7.4f, %7.4f]\n",
		v->tv.x, v->tv.y, v->tv.z, v->tv.w);
}

void
main(int argc, char *argv[]) {
	p3dc_XFRM T1, T2, *A, *B;
	p3dc_VRTX	*V, V2;

	p3dc_identity_xfrm(&T1);
	p3dc_identity_xfrm(&T2);

	p3dc_xrot_xfrm(&T1, 45);
	p3dc_yrot_xfrm(&T2, 45);
	A = p3dc_mult_xfrm(NULL, &T1, &T2);
	B = p3dc_mult_xfrm(NULL, &T2, &T1);
	V = p3dc_new_vrtx(0, 0, 1.0);
	printf("Rotation in two steps.\n");
	p3dc_xfrm_vrtx(&V2, &T1, V);
	dump_vrtx("First rotation", &V2);
	p3dc_xfrm_vrtx(NULL, &T2, &V2);
	dump_vrtx("Second Rotation", &V2);
	p3dc_xfrm_vrtx(NULL, A, V);
	dump_vrtx("T1 * T2", V);
	p3dc_xfrm_vrtx(NULL, B, V);
	dump_vrtx("T2 * T1", V);
	exit(0);
}
