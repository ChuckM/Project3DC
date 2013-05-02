

/*% cc -g sphere.c -o sphere -lm
 *
 * sphere - generate a triangle mesh approximating a sphere by
 *  recursive subdivision. First approximation is an octahedron;
 *  each level of refinement increases the number of triangles by
 *  a factor of 4.
 * Level 3 (128 triangles) is a good tradeoff if gouraud
 *  shading is used to render the database.
 *
 * Usage: sphere [level] [-p] [-c]
 *	level is an integer >= 1 setting the recursion level (default 1).
 *	-p causes generation of a PPHIGS format ASCII archive
 *	    instead of the default generic output format.
 *	-c causes triangles to be generated with vertices in counterclockwise
 *	    order as viewed from the outside in a RHS coordinate system.
 *	    The default is clockwise order.
 *
 *  The subroutines print_object() and print_triangle() should
 *  be changed to generate whatever the desired database format is.
 *
 * Jon Leech (leech@cs.unc.edu) 3/24/89
 */
#include <stdio.h>
#include <math.h>

typedef struct {
    double  x, y, z;
} point;

typedef struct {
	double u, v;
} texture;

typedef struct {
    point		pt[3];	/* Vertices of triangle */
	texture		tx[3];	/* Texture co-ordinates */
} triangle;

typedef struct {
    int       npoly;	/* # of triangles in object */
    triangle *poly;	/* Triangles */
} object;

static vtx_num = 1;
struct vtx_list {
	char name[8];
	double x, y, z;
	struct vtx_list *nxt;
};

static struct vtx_list *head = NULL;

/* Six equidistant points lying on the unit sphere */
#define XPLUS {  1,  0,  0 }	/*  X */
#define XMIN  { -1,  0,  0 }	/* -X */
#define YPLUS {  0,  1,  0 }	/*  Y */
#define YMIN  {  0, -1,  0 }	/* -Y */
#define ZPLUS {  0,  0,  1 }	/*  Z */
#define ZMIN  {  0,  0, -1 }	/* -Z */
#ifdef ONE_TEXTURE
#define UPPER_BOT_LEFT	{ 0.5, 127.5 }	/* Bottom left of texture */
#define UPPER_BOT_RIGHT { 127.5, 127.5 }
#define UPPER_TOP_LEFT	{ 0.5, 0.5 }
#define	UPPER_TOP_RIGHT { 127.5, 0.5 }
#define UPPER_CENTER { 63.5, 63.5 }
#define LOWER_BOT_LEFT	{ 128.5, 127.5 }
#define LOWER_BOT_RIGHT	{ 128.5, 255.5 }
#define LOWER_TOP_LEFT	{ 128.5, 0.5 }
#define LOWER_TOP_RIGHT	{ 128.5, 255.5 }
#define LOWER_CENTER 	{ 192.5, 63.5 }
#else
#define UPPER_BOT_LEFT	{ 0.5, 255.5 }	/* Bottom left of texture */
#define UPPER_BOT_RIGHT { 255.5, 255.5 }
#define UPPER_TOP_LEFT	{ 0.5, 0.5 }
#define	UPPER_TOP_RIGHT { 255.5, 0.5 }
#define UPPER_CENTER 	{ 127.5, 127.5 }

#define LOWER_BOT_LEFT	{ 0.5, 255.5 }
#define LOWER_BOT_RIGHT	{ 255.5, 255.5 }
#define LOWER_TOP_LEFT	{ 0.5, 0.5 }
#define LOWER_TOP_RIGHT	{ 255.5, 0.5 }
#define LOWER_CENTER 	{ 127.5, 127.5 }
#endif

/* Vertices of a unit octahedron */
triangle octahedron[] = {
    { { XPLUS, ZPLUS, YPLUS }, {UPPER_TOP_RIGHT, UPPER_TOP_LEFT,  UPPER_CENTER}},
    { { YPLUS, ZPLUS, XMIN  }, {UPPER_CENTER, UPPER_TOP_LEFT, UPPER_BOT_LEFT }},
    { { XMIN , ZPLUS, YMIN  }, {LOWER_TOP_RIGHT, LOWER_TOP_LEFT, LOWER_CENTER} },
    { { YMIN , ZPLUS, XPLUS }, {LOWER_CENTER, LOWER_TOP_LEFT, LOWER_BOT_LEFT }},
    { { XPLUS, YPLUS, ZMIN  }, {UPPER_TOP_RIGHT, UPPER_CENTER, UPPER_BOT_RIGHT} },
    { { YPLUS, XMIN , ZMIN  }, {UPPER_CENTER, UPPER_BOT_LEFT, UPPER_BOT_RIGHT }},
    { { XMIN , YMIN , ZMIN  }, {LOWER_TOP_RIGHT, LOWER_CENTER, LOWER_BOT_RIGHT }},
    { { YMIN , XPLUS, ZMIN  }, {LOWER_CENTER, LOWER_BOT_LEFT, LOWER_BOT_RIGHT }}
};

/* A unit octahedron */
object oct = {
    sizeof(octahedron) / sizeof(octahedron[0]),
    &octahedron[0]
};

int PPHIGSflag = 0;

/* Forward declarations */
point *normalize(/* point *p */);
point *midpoint(/* point *a, point *b */);
texture *midtex(/* point *a, point *b */);
void print_object(/* object *obj, int level */);
void print_triangle(/* triangle *t, int */);
void pphigs_header(/* int */);
void pphigs_trailer();

extern char *malloc(/* unsigned */);

main(ac, av)
int ac;
char *av[];
{
    object *old, *new;
    int     ccwflag = 0,	/* Reverse vertex order if true */
	i,
	level,		/* Current subdivision level */
	maxlevel = 1;	/* Maximum subdivision level */

    /* Parse arguments */
    for (i = 1; i < ac; i++) {
		if (!strcmp(av[i], "-p"))
			PPHIGSflag = 1;
		else if (!strcmp(av[i], "-c"))
			ccwflag = 1;
		else if ((maxlevel = atoi(av[i])) < 1) {
			fprintf(stderr, "%s: # of levels must be >= 1\n", av[0]);
			exit(1);
		}
	}

	if (ccwflag) {
		/* Reverse order of points in each triangle */
		for (i = 0; i < oct.npoly; i++) {
			point tmp;
			tmp = oct.poly[i].pt[0];
			oct.poly[i].pt[0] = oct.poly[i].pt[2];
			oct.poly[i].pt[2] = tmp;
		}
	}

	old = &oct;

	/* Subdivide each starting triangle (maxlevel - 1) times */
	for (level = 1; level < maxlevel; level++) {
		/* Allocate a new object */
		new = (object *)malloc(sizeof(object));
		if (new == NULL) {
			fprintf(stderr, "%s: Out of memory on subdivision level %d\n",
									av[0], level);
			exit(1);
		}
		new->npoly = old->npoly * 4;

		/* Allocate 4* the number of points in the current approximation */
		new->poly  = (triangle *)malloc(new->npoly * sizeof(triangle));
		if (new->poly == NULL) {
			fprintf(stderr, "%s: Out of memory on subdivision level %d\n",
									av[0], level);
			exit(1);
		}

	/* Subdivide each triangle in the old approximation and normalize
	 *  the new points thus generated to lie on the surface of the unit
	 *  sphere.
	 * Each input triangle with vertices labelled [0,1,2] as shown
	 *  below will be turned into four new triangles:
	 *
	 *			Make new points
	 *			    a = (0+2)/2
	 *			    b = (0+1)/2
	 *			    c = (1+2)/2
	 *       1
	 *       /\		Normalize a, b, c
	 *      /  \
	 *    b/____\ c		Construct new triangles
	 *    /\    /\		    [0,b,a]
	 *   /	\  /  \		    [b,1,c]
	 *  /____\/____\	    [a,b,c]
	 * 0	  a	2	    [a,c,2]
	 */
	for (i = 0; i < old->npoly; i++) {
	    triangle
		 *oldt = &old->poly[i],
		 *newt = &new->poly[i*4];
	    point a, b, c;
		texture ta, tb, tc;

	    a = *normalize(midpoint(&oldt->pt[0], &oldt->pt[2]));
	    b = *normalize(midpoint(&oldt->pt[0], &oldt->pt[1]));
	    c = *normalize(midpoint(&oldt->pt[1], &oldt->pt[2]));
		ta = *midtex(&oldt->tx[0], &oldt->tx[2]);
		tb = *midtex(&oldt->tx[0], &oldt->tx[1]);
		tc = *midtex(&oldt->tx[1], &oldt->tx[2]);

	    newt->pt[0] = oldt->pt[0];
		newt->tx[0] = oldt->tx[0];
	    newt->pt[1] = b;
		newt->tx[1] = tb;
	    newt->pt[2] = a;
		newt->tx[2] = ta;
	    newt++;

	    newt->pt[0] = b;
		newt->tx[0] = tb;
	    newt->pt[1] = oldt->pt[1];
		newt->tx[1] = oldt->tx[1];
	    newt->pt[2] = c;
		newt->tx[2] = tc;
	    newt++;

	    newt->pt[0] = a;
		newt->tx[0] = ta;
	    newt->pt[1] = b;
		newt->tx[1] = tb;
	    newt->pt[2] = c;
		newt->tx[2] = tc;
	    newt++;

	    newt->pt[0] = a;
		newt->tx[0] = ta;
	    newt->pt[1] = c;
		newt->tx[1] = tc;
	    newt->pt[2] = oldt->pt[2];
		newt->tx[2] = oldt->tx[2];
	}

	if (level > 1) {
	    free(old->poly);
	    free(old);
	}

	/* Continue subdividing new triangles */
	old = new;
    }

    /* Print out resulting approximation */
    print_object(old, maxlevel);
}

/* Normalize a point p */
point *normalize(p)
point *p;
{
    static point r;
    double mag;

    r = *p;
    mag = r.x * r.x + r.y * r.y + r.z * r.z;
    if (mag != 0.0) {
	mag = 1.0 / sqrt(mag);
	r.x *= mag;
	r.y *= mag;
	r.z *= mag;
    }

    return &r;
}

/* Return the midpoint on the line between two points */
point *midpoint(a, b)
point *a, *b;
{
    static point r;

    r.x = (a->x + b->x) * 0.5;
    r.y = (a->y + b->y) * 0.5;
    r.z = (a->z + b->z) * 0.5;

    return &r;
}

/* texture mid point */
texture *midtex(texture *a, texture *b) {
	static texture r;

    r.u = (a->u + b->u) * 0.5;
    r.v = (a->v + b->v) * 0.5;
    return &r;
}

struct vtx {
	char name[16];
	double x, y, z;
	struct vtx *nxt;
};

/*
 * Recursively print the vertex list (reverse the order
 * back into the original vertex order.
 */
void
print_vertex(struct vtx_list *v) {
	if (v == NULL) return;
	print_vertex(v->nxt);
	printf("	Vertex %s = %g,  %g,  %g ;\n", v->name, 
			v->x, v->y, v->z);
}

/* Write out all triangles in an object */
void print_object(obj, level)
object *obj;
int level;
{
    int i;

	printf("#\n# Sphere Model - autogenerated %d levels\n", level);
	printf("#\n# This model was generated automatically by a program\n");
	printf("# sphere.c that works by starting with an octagon (level 1)\n");
	printf("# and then subdividing each triangle into four triangles.\n");
	printf("# The generated triangles are then 'pushed' out to the \n");
	printf("# unit sphere, creating new triangles on the surface of the\n");
	printf("# generated sphere.\n#\n");
	printf("model sphere%d {\n", level);
	printf("	origin 0, 0, 0;\n");
	printf("	color base = 200, 200, 200 ;\n");
	printf("	texture top_texture = decal ;\n");
	printf("	texture bottom_texture = decal ;\n");
    if (PPHIGSflag)
		pphigs_header(level);

    /* Spit out coordinates for each triangle */
    for (i = 0; i < obj->npoly; i++)
		print_triangle(&obj->poly[i], 0);

	print_vertex(head);

    for (i = 0; i < obj->npoly; i++)
		print_triangle(&obj->poly[i], 1);

    if (PPHIGSflag)
		pphigs_trailer();
	printf("}\n");
}

int FEQ(double x, double y) {
	return (fabs(x - y) < .001);
}

static char *
name_is(point *p) {
	struct vtx_list *vt;
			
	for (vt = head; vt != NULL; vt = vt->nxt) {
		if (FEQ(vt->x, p->x) && 
			FEQ(vt->y, p->y) &&
			FEQ(vt->z, p->z)) {
			return &(vt->name[0]);
		}
	}
	vt = (struct vtx_list *)malloc(sizeof(struct vtx_list));
	vt->x = p->x; vt->y = p->y; vt->z = p->z;
	sprintf(vt->name, "v%03d", vtx_num++);
	vt->nxt = head;
	head = vt;
	return &(vt->name[0]);
}

/* Output a triangle */
void print_triangle(t, do_print)
triangle *t;
int do_print;
{
	char *v1, *v2, *v3;
    int i;

    if (PPHIGSflag) {
	printf("\tpolygon 3 {\n");
	for (i = 0; i < 3; i++)
	    printf("\t\t%g\t%g\t%g %g\t%g\t%g ;\n",
		t->pt[i].x, t->pt[i].y, t->pt[i].z,	/* Point */
		t->pt[i].x, t->pt[i].y, t->pt[i].z);	/* Normal */
	printf("\t};\n");
	} else {
		/* Modify this to generate your favorite output format
		 * Triangle vertices are in t->pt[0..2].{x,y,z}
		 * A generic format is provided.
		 */
		if (do_print) {
			int not_top = 
				(t->pt[0].y < 0) || (t->pt[1].y < 0) || (t->pt[2].y < 0);
			printf("	Face <%s> (base) ", 
							not_top ? "bottom_texture" : "top_texture");
		}
		v1 = name_is(&(t->pt[0]));
		v2 = name_is(&(t->pt[1]));
		v3 = name_is(&(t->pt[2]));
		if (do_print) {
			printf(" %s [%g, %g, %g] <%g, %g>,", v1, 
					t->pt[0].x, t->pt[0].y, t->pt[0].z,
					t->tx[0].u, t->tx[0].v);
			printf(" %s [%g, %g, %g] <%g, %g>,", v2, 
					t->pt[1].x, t->pt[1].y, t->pt[1].z,
					t->tx[1].u, t->tx[1].v);
			printf(" %s [%g, %g, %g] <%g, %g>;\n", v3, 
					t->pt[2].x, t->pt[2].y, t->pt[2].z,
					t->tx[2].u, t->tx[2].v);
		}
    }
}

/* Generate header for a PPHIGS ASCII archive */
void pphigs_header(level)
int level;
{
    int dx, dy, dz;

    printf("structure sphere%d posted {\n", level);
    printf("\tcolor polygon {\n");
    printf("\t\t200 100  50   0     50 100 200   0\n");
    printf("\t};\n");

    switch (level) {
	case 1:
	    dx = -2000; dy =  2000; dz = 0;
	    break;
	case 2:
	    dx =  2000; dy =  2000; dz = 0;
	    break;
	case 3:
	    dx = -2000; dy = -2000; dz = 0;
	    break;
	case 4:
	    dx =  2000; dy = -2000; dz = 0;
	    break;
	case 5:
	    dx =     0; dy =	 0; dz = 0;
	    break;
	default:
	    dx = dy = dz = 0;
	    break;
    }

    printf("\tmatrix Pre scale 1000 1000 1000;\n");
    printf("\tmatrix Pre translate %d %d %d ;\n", dx, dy, dz);
}

void pphigs_trailer() {
    printf("};\n");
}
