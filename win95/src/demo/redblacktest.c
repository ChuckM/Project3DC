/*
 * A simple test for the Red/Black tree code.
 *
 * This test inserts a number of nodes into a red/black list
 * and then prints them out in the P3DC log.
 */

#include <stdio.h>
#include <stdlib.h>
#include "p3dc.h"

char *(colors[]) = {
		"red",
		"purple",
		"orange",
		"white",
		"black",
		"magenta",
		"peusse",
		"violet",
		"yellow",
		"green",
		"mango",
		"blue",
		"indigo",
		"teal",
		"cardinal",
		"turquoise",
		NULL,
};

/*
 * When removed, these node exercise the four interesting test cases.
 */
char *(tests[]) = {
		"peusse",
		"orange",
		"green",
		"violet",
		"purple",
		NULL,
};

extern int p3dc_rbinsert_node(p3dc_LIST *, p3dc_NODE *);
extern void p3dc_rbprint_list(p3dc_LIST *);

void
outlist(p3dc_LIST *list) {
	int i = 0;
	p3dc_NODE *n;

	for (n = p3dc_rbfirst_node(list); n != NULL; n = p3dc_rbnext_node(list)) {
		p3dc_log("%d - %s\n", i+1, n->name);
		i++;
	}
}

void
main(int argc, char *argv[]) {
	p3dc_NODE *n;
	p3dc_LIST	nodelist = {NULL, NULL, P3DC_UNKNOWN, NULL };
	int i;

	i = 0;
	p3dc_log_open("test.txt");
	p3dc_log("Node test start.\n");
	while (colors[i] != NULL) {
		n = p3dc_new_node(NULL);
		if (n) {
			n->name = colors[i];
			p3dc_log("Inserting node %s\n", colors[i]);
			p3dc_rbinsert_node(&nodelist, n);
		}
		i++;
	}
	p3dc_rbprint_list(&nodelist);
	outlist(&nodelist);
	i = 0;
	while(tests[i] != NULL) {
		n = p3dc_rblocate_node(&nodelist, tests[i]);
		if (n == NULL) {
			p3dc_log("TEST FAILED. Could not locate %s\n", tests[i]);
			continue;
		}
		p3dc_rbremove_node(&nodelist, n);
		p3dc_log("Deleted %s. New list :\n", tests[i]);
		p3dc_rbprint_list(&nodelist);
		outlist(&nodelist);
		// p3dc_free_node(n);
		i++;
	}
	exit(0);
}