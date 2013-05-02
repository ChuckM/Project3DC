/*
 * This file tests the parsing of the model code in G3D
 */
#include <stdio.h>
#include "g3d.h"

void
main(int argc, char *argv[]) {
	g3d_MODEL *m;

	m = g3d_new_model("test", 3.0f);
	if (m != NULL)
		g3d_dump_model(m);
}