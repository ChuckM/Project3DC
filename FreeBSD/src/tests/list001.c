/*
 * list001.c - basic function test for lists.
 */

#include <stdio.h>
#include <stdlib.h>
#include <p3dc.h>

char *(payloads[]) =  {
	"this",
	"brown",
	"fox",
	"jumped",
	"over",
	"the",
	"lazy",
	"dog.",
	NULL
};
	
void
main(int argc, char *argv[]) {
	int	i;
	/* char *s; */
	p3dc_LIST *link = p3dc_new_list(P3DC_UNKNOWN, P3DC_LIST_LINKED);
	p3dc_LIST other;
	p3dc_NODE *n;

	if (link == NULL) {
		fprintf(stderr, "Couldn't allocate a list.\n");
		exit(1);
	}

	for (i = 0; payloads[i] != NULL; i++) {
		n = p3dc_new_node(payloads[i], NULL, 0);
		if (n == NULL) {
			fprintf(stderr, "Couldn't allocate node %d.\n", i+1);
			exit(1);
		}
		p3dc_add_node(link, n, P3DC_LIST_TAIL);
	}

	for (n = FIRST_NODE(link); n != NULL; n = NEXT_NODE(link)) {
		printf("Node : %s\n", (char *)(n->data.p));
	}

	p3dc_init_list(&other, P3DC_UNKNOWN, P3DC_LIST_LINKED);
	while ((n = p3dc_remove_node(link, P3DC_NODE_FIRST)) != NULL) {
		p3dc_add_node(&other, n, P3DC_LIST_HEAD);
	}

	for (n = FIRST_NODE(&other); n != NULL; n = NEXT_NODE(&other)) {
		printf("Node : %s\n", (char *)(n->data.p));
	}

	exit(0);
}
