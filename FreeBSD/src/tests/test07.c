#include <stdio.h>
#include <p3dc.h>

char *(colors[]) = {
	"red",
	"green",
	"purple",
	"orange",
	"violet",
	"brown",
	"black",
	"chartrueuse",
	"indigo",
	"blue",
	"teal",
	"sand",
	"white",
	"hazel",
	"turquoise",
	"wheat",
	"whey",
	"wow",
	"wren",
	"water",
	"willow",
	"tin",
	"silly",
	"submarine",
	"aquamarine",
	NULL
};

void
main(int argc, char *argv[]) {
	p3dc_NODE *n;
	p3dc_LIST nodes;
	int i;

	i = 0;
	p3dc_init_list(&nodes, P3DC_UNKNOWN, P3DC_LIST_SORTED);
	p3dc_log_open("/dev/tty");
	while (colors[i]) {
		n = p3dc_new_node(NULL, colors[i], 0);
		printf("Inserting node %s\n", colors[i]);
		if (p3dc_add_node(&nodes, n, P3DC_LIST_BYNAME, n->name)) {
			fprintf(stderr, "Error inserting node %s\n", colors[i]);
		}
		i++;
	}
	p3dc_rbprint_list(&nodes);
	i =0;
	for (n = FIRST_NODE(&nodes); n != NULL; n = NEXT_NODE(&nodes)) {
		p3dc_log("%02d : %s\n", i+1, n->name);
		i++;
	}
	exit(0);
}
