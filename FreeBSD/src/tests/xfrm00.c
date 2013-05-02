#include <stdio.h>
#include <p3dc.h>

extern void p3dc_dump_xfrm();

void
main(int argc, char *argv[]) {
	p3dc_XFRM T;

	p3dc_identity_xfrm(&T);
	p3dc_log_open("/dev/tty");
	T.data[0][0] = 0;
	T.data[0][1] = 1;
	T.data[0][2] = 2;
	T.data[0][3] = 3;
	T.data[1][0] = 10;
	T.data[1][1] = 11;
	T.data[1][2] = 12;
	T.data[1][3] = 13;
	T.data[2][0] = 20;
	T.data[2][1] = 21;
	T.data[2][2] = 22;
	T.data[2][3] = 23;
	T.data[3][0] = 30;
	T.data[3][1] = 31;
	T.data[3][2] = 32;
	T.data[3][3] = 33;
	p3dc_dump_xfrm(&T, NULL, NULL, NULL, NULL);
}
