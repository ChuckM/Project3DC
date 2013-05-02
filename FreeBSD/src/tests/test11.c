#include <stdio.h>
#include <p3dc.h>

extern void p3dc_dump_xfrm();

void
main(int argc, char *argv[]) {
	char *head = "*HEAD*  ";
	char *tail = "  *TAIL*\n";
	char *title = "*TITLE*TITLE*\n";
	char *fmt = "%g";
	p3dc_XFRM *t;
	char	logname[32];

	sprintf(logname, "%s.log", argv[0]);
	p3dc_log_open(logname);
	t = p3dc_identity_xfrm(NULL);
	p3dc_dump_xfrm(t, title, head, fmt, tail);
	p3dc_log("\n\n");
	p3dc_dump_xfrm(t, NULL, NULL, NULL, NULL);
	p3dc_log_close();
}
