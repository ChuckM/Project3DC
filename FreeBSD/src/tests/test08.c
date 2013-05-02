/*
 * Attempt to parse a model and then to undump it to
 * standard out.
 */
#include <stdio.h>
#include "p3dc.h"

void
main(int argc, char *argv[]) {
	p3dc_MODEL *m;

	p3dc_log_open("/dev/tty");
	m = p3dc_new_model("cube", 1.0, 1.0, 1.0);
	if (m) {
		p3dc_rbprint_list(&(m->parts));
		p3dc_dump_model(m); 
	} else 
		p3dc_log("Didn't successfully read the model.\n");
}
