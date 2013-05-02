/*
 * p3dc.h -- Graphics Library Include File
 * 
 * Copyright statement:
 *
 * All of the documentation and software included in the Generic
 * 3D library is copyrighted by Chuck McManis.
 *
 *   Copyright (c) 1997, 1998 Chuck McManis, all rights
 *   reserved. 
 * 
 * Redistribution and use in source and binary forms, with or
 * without modification, is permitted provided that the terms
 * specified in the file license.txt are met.
 * 
 * THIS SOFTWARE IS PROVIDED BY CHUCK MCMANIS "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL CHUCK MCMANIS BE LIABLE FOR AND
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * Change history:
 *	$Log: p3dc.h,v $
 *	Revision 1.6  1999-10-31 11:23:55-08  cmcmanis
 *	Camera rewrite is nearly complete. This seems like a logical
 *	milestone
 *
 *	Revision 1.5  1999/09/12 06:32:15  cmcmanis
 *	updated clipping API (again?)
 *
 *	Revision 1.3  1999/09/09 02:39:41  cmcmanis
 *	Added protos for memstats
 *
 *	Revision 1.2  1999/09/02 03:22:09  cmcmanis
 *	The great LIST rewrite, lists are much cleaner now
 *
 *	Revision 1.1  1999/08/18 01:33:10  cmcmanis
 *	Initial revision
 *
 *	Revision 1.1  1999/08/18 01:05:20  cmcmanis
 *	Initial revision
 *
 *	Revision 1.17  1999/08/14 23:19:56  cmcmanis
 *	This version now has the new model support. It allows things to
 *	set the colors and textures of a model after it has been instantiated
 *
 *	Revision 1.16  1999/07/26 03:09:47  cmcmanis
 *	Renaming is essentially complete, modelTest runs again
 *
 *	Revision 1.15  1998/11/26 08:05:03  cmcmanis
 *	Added prototype for rbremove
 *
 *	Revision 1.14  1998/11/26 02:10:21  cmcmanis
 *	Recovered p3dc.h
 *
 *	Revision 1.12  1998/11/08 05:59:31  cmcmanis
 *	Outdated (renamed to 'H:\P3DC-RCS\src\include\p3dc.h,v').
 *
 *	Revision 1.12  1998/11/08 05:23:06  cmcmanis
 *	Outdated (renamed to 'H:\P3DC-RCS\include\p3dc.h,v').
 *
 *	Revision 1.11  1998/11/05 20:23:48  cmcmanis
 *	Checkin prior to moving into a better source tree structure
 *
 *	Revision 1.10  1998/10/18 09:29:42  cmcmanis
 *	Models now position and move correctly. I've re-ordered my matrices
 *	to be SPHIGs compatible so that I can use SPHIGS to test my results.
 *	Created p3dc_camera_xfrm and p3dc_orientation_xfrm as compliments of
 *	each other. Cool stuff there.
 *
 *	Revision 1.9  1998/10/13 21:04:12  cmcmanis
 *	Model parsing and basic drawing is checked out. The model positioning
 *	code and relighting code still needs work.
 *
 *	Revision 1.8  1998/10/10 07:00:33  cmcmanis
 *	Model parsing is written (not fully debugged) and the texture structures
 *	are closer to their final form. By the next check in we should have a
 *	few simple models up and running with textured models to follow.
 *
 *	Revision 1.7  1998/09/23 08:48:47  cmcmanis
 *	Clipping works! Polygon and line clipping is pretty robust at this point
 *	still looking for corner cases but I believe all of the code is correct at
 *	this point.
 *
 *	Revision 1.6  1998/09/20 08:10:49  cmcmanis
 *	Major rewrite, added list stuff, and primitives. Almost completely
 *	back to where version 4 was at this point. This version is ALPHA0_2
 *
 *	Revision 1.5  1998/09/18 08:34:37  cmcmanis
 *	This update has new clipping code for polygons.
 *	In particular the nodes are not needlessly copied.
 *
 *	Adds the new list capability in util.c/memory.c that
 *	uses a p3dc_NLST (type P3DC_LIST) as a simple
 *	linked list.
 *
 *	Revision 1.4  1998/09/08 04:04:55  cmcmanis
 *	Clipping for lines and points is now verified, allocation
 *	of vertices in the GAL layer is fixed.
 *
 *	Revision 1.3  1998/09/04 05:47:51  cmcmanis
 *	Lighting works for lines and points.
 *	New memory allocator is in place but not
 *	heavily used yet.
 *
 *	Revision 1.2  1998/09/03 19:52:05  cmcmanis
 *	Cube Test runs, camera and transformation code checks out,
 *  lighting still does not work correctly.
 *
 *	Revision 1.1  1998/09/02 08:48:19  cmcmanis
 *	Initial revision
 *
 *	Revision 1.3  1998/08/27 07:56:32  cmcmanis
 *	New clip architecture, added the NLST allocator/deallocator
 *
 *	Revision 1.2  1998/08/25 16:12:54  cmcmanis
 *	Basic compile working, built with new p3dc.h.
 *
 *	Revision 1.1  1998/08/23 22:03:00  cmcmanis
 *	Initial revision
 */

#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN
#endif

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	B A S E   T Y P E   D E F I N I T I O N S

  The base types are simple types that are used later in the compound
  types to build a more sophisticated structure. The only ones that are
  manipulated directly are the PNT3 -> PNT4 via transformation.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*
 * FLOAT - Defines the basic floating point type for the library. 
 * Initially it was 'double' but that is a bit slow. Further, float 
 * works well when the displays are 800 x 600 or less, more than that
 * and you can start seeing artifacts.
 */
typedef float p3dc_FLOAT;

/*
 * POINT2 - This is a two co-ordinate point (used for textures) 
 * hence it is a u,v space. (rather than an x,y space)
 */
typedef struct p3dc_point2 {
	p3dc_FLOAT	u, v;
} p3dc_PNT2;

/* A fast macro that will multiply a PNT2 by a scalar value */
#define scaleP2(p3, f) {\
	(p3)->u = (p3)->u * (p3dc_FLOAT)(f); \
	(p3)->v = (p3)->v * (p3dc_FLOAT)(f); \
}

/* An inline define that computes a PNT2 Intercept. */
#define interceptP2(a, b, dt, res) { \
	(res)->u = (a)->u + (dt) * ((b)->u - (a)->u); \
	(res)->v = (a)->v + (dt) * ((b)->v - (a)->v); \
}

/*
 * POINT3 - This is a point in three space represented by three
 * floating point values.
 */
typedef struct p3dc_point3 {
	p3dc_FLOAT	x, y, z; 
} p3dc_PNT3;

/* A fast macro that will multiply a PNT3 by a scalar value */
#define scaleP3(p3, f) {\
	(p3)->x = (p3)->x * (p3dc_FLOAT)(f); \
	(p3)->y = (p3)->y * (p3dc_FLOAT)(f); \
	(p3)->z = (p3)->z * (p3dc_FLOAT)(f); \
}

/* An inline define that computes a PNT4 Intercept. */
#define interceptP3(a, b, dt, res) { \
	(res)->x = (a)->x + (dt) * ((b)->x - (a)->x); \
	(res)->y = (a)->y + (dt) * ((b)->y - (a)->y); \
	(res)->z = (a)->z + (dt) * ((b)->z - (a)->z); \
}

/*
 * POINT4 - This is usually a point in 3 space that has been 
 * transformed into a point in fourspace by a 4 x 4 transformation
 * matrix.
 */
typedef struct p3dc_point4 {
	p3dc_FLOAT x, y, z, w;
} p3dc_PNT4;

/* A fast macro that will multiply a PNT4 by a scalar value */
#define scaleP4(p4, f) {\
	(p4)->x = (p4)->x * (p3dc_FLOAT)(f); \
	(p4)->y = (p4)->y * (p3dc_FLOAT)(f); \
	(p4)->z = (p4)->z * (p3dc_FLOAT)(f); \
	(p4)->w = (p4)->w * (p3dc_FLOAT)(f);\
}

/* An inline define that computes a PNT4 Intercept. */
#define interceptP4(a, b, dt, res) { \
	(res)->x = (a)->x + (dt) * ((b)->x - (a)->x); \
	(res)->y = (a)->y + (dt) * ((b)->y - (a)->y); \
	(res)->z = (a)->z + (dt) * ((b)->z - (a)->z); \
	(res)->w = (a)->w + (dt) * ((b)->w - (a)->w); \
}

/*
 * XFRM - This is a 4 x 4 matrix that is used to transform points in
 * 3d space. I've chosen to use a 4 x 4 matrix in order to get both
 * translation and rotation in a single matrix. The decision to stick
 * with a 4 x 4 matrix does waste space (about 4 * sizeof(float))
 * however it doesn't waste time, because for matrices where 3,3 == 1.0
 * I do the computation as a 3 x 3 matrix plus and add.
 */
typedef struct p3dc_xfrm {
	unsigned short	id;		/* Unique identity of this transform */
	p3dc_FLOAT	data[4][4];	/* 4 x 4 matrix of values for this transform */
} p3dc_XFRM;


/* Type definitions for the NODE lists */
typedef enum p3dc_type { 
	  P3DC_UNKNOWN = 0,
  	  P3DC_NOTYPE, 
	  P3DC_FLOAT,
	  P3DC_PNT2,
	  P3DC_PNT3,
	  P3DC_PNT4,
	  P3DC_XFRM,
	  P3DC_TDATA,
	  P3DC_TXTR,
	  P3DC_CLR,
	  P3DC_VRTX,
	  P3DC_FACE,
	  P3DC_CMRA,
	  P3DC_MODEL,
	  P3DC_TYPE,
	  P3DC_LIST,
	  P3DC_NODE,
	  P3DC_LIGHT,
	  P3DC_POINT,
	  P3DC_LINE,
	  P3DC_POLY,
} p3dc_TYPE;

#define P3DC_NODE_BLACK			1	/* Node color is black */
#define P3DC_NODE_RED			2	/* Node color is red */
#define P3DC_NODE_FREEPAYLOAD	4	/* Payload should be freed when node is freed */
#define P3DC_NODE_LLNODE		8	/* Node should be on a linked list */

typedef enum __node_location {
	P3DC_NODE_CURRENT =	0,	/* Get current node */
	P3DC_NODE_FIRST,		/* Get first node on list */
	P3DC_NODE_NEXT,			/* Get next node on list */
	P3DC_NODE_LAST,			/* Get last node on the list */
	P3DC_NODE_PREVIOUS,		/* Get previous node on the list */
	P3DC_NODE_BYNAME,		/* Get named node on the list */
	P3DC_NODE_BYDATA,		/* Get node with matching payload */
	P3DC_NODE_THIS			/* Remove this node from the list */
} p3dc_NODE_LOCATION; 

typedef enum __node_position {
	P3DC_LIST_HEAD = 0,		/* Insert at the head of the list */
	P3DC_LIST_TAIL,			/* Insert at the end of the list */
	P3DC_LIST_BEFORE,		/* Insert before the specified node */
	P3DC_LIST_AFTER,		/* Insert after specified node */
	P3DC_LIST_BYNAME		/* Insert by name */
} p3dc_NODE_POSITION;

typedef enum _list_type {
	P3DC_LIST_LINKED = 0,		/* Linked list */
	P3DC_LIST_SORTED			/* Searchable list */
} p3dc_LIST_TYPE;


/*
 * LISTNODE - This is a utility linkage structure. It is used for 
 * linking lists of shared data structures (such as vertices).
 */
typedef struct __p3dc_node {
	struct {
		void			*p;		/* Pointer to the node data */
		p3dc_TYPE		t;		/* Node's object data type */
	}					data;
	struct __p3dc_node	*nxt;
	struct __p3dc_node	*prv;
	struct __p3dc_node	*parent; /* Parent node in BST lists */
	struct __p3dc_list	*owner;	/* Pointer to the list on which this node lives */
	char				*name;	/* Optional 'name' if in a red/black tree */
	int					flags;	/* Housekeeping Flags */
} p3dc_NODE;

/* 
 * LIST = This is a utility linkage structure that collects
 * nodes into a doubly linked list.
 */
typedef struct __p3dc_list {
	p3dc_NODE	*head;		/* First node in the list */
	p3dc_NODE	*tail;		/* Last node in the list */
	p3dc_TYPE	n_type;		/* The types of things on this list */
	p3dc_LIST_TYPE	l_type;	/* Type of list (single, double, bst, ...) */
	p3dc_NODE	*current;	/* Current node (for iterating) */
} p3dc_LIST;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	C O L O R    D E F I N I T I O N S

  P3DC uses a 24bit RGBA color model. This is supported natively on VooDoo
  Graphics which is nice. The only tricky part here is that the layout
  in memory is different between little endian (Intel) and big endian
  (PowerPC) systems. The defines here thus need to know which kind of
  system you are using. It defaults to Intel and LITTLE ENDIAN.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifdef LITTLE_ENDIAN
/* These are for the Intel (little endian) architecture! */
#define p3dc_CLR_RED		3	/* Index to red component */
#define p3dc_CLR_GREEN	2	/* Index to green component */
#define p3dc_CLR_BLUE	1	/* Index to blue component */
#define p3dc_CLR_ALPHA	0	/* Index to alpha component */
#else
/* These are for the PowerPC (big endian) architecture! */
#define p3dc_CLR_RED		0	/* Index to red component */
#define p3dc_CLR_GREEN	1	/* Index to green component */
#define p3dc_CLR_BLUE	2	/* Index to blue component */
#define p3dc_CLR_ALPHA	3	/* Index to alpha component */
#endif

/*
 * This structure holds and 32 bit RGBA color.
 */
typedef union p3dc_color {
	unsigned long rgba;
	unsigned char c[4];
} p3dc_CLR;				/* component's color */

#define gRED	c[p3dc_CLR_RED]
#define gGREEN	c[p3dc_CLR_GREEN]
#define gBLUE	c[p3dc_CLR_BLUE]
#define gALPHA	c[p3dc_CLR_ALPHA]

/*
 * These define some generic colors, (implemented in util.c)
 */
extern p3dc_CLR COLOR_black;
extern p3dc_CLR COLOR_red;
extern p3dc_CLR COLOR_green;
extern p3dc_CLR COLOR_yellow;
extern p3dc_CLR COLOR_blue;
extern p3dc_CLR COLOR_purple;
extern p3dc_CLR COLOR_cyan;
extern p3dc_CLR COLOR_white;
extern p3dc_CLR COLOR_lt_gray;
extern p3dc_CLR COLOR_md_gray;
extern p3dc_CLR COLOR_dk_gray;
extern p3dc_CLR COLOR_brown;
extern p3dc_CLR COLOR_orange;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	C O M P O U N D   T Y P E   D E F I N I T I O N S

  These are the compound type definitions. They are called compound
  definitions because they are composed of the base types that are 
  defined above. As a programmer that means that if you change a base
  type you need to look here to see if you need to update the compound
  definition. Also the compound definitions are the "objects" that are
  used by the library most.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	T E X T U R E   D E F I N I T I O N

  Textures are the first of the compound data structure definitions. My
  texture structure includes an opaque pointer to a structure that the
  underlying hardware texture code understands. Then there are a set of
  co-ordinates. As a texture is always associated with a FACE structure,
  (see below) the number of co-ordinate pairs is constrained to be the
  same as the number of vertices in the face structure.

  There are currently three types of textures defined, decals, light maps,
  and details. Texture structures are designed such that the pointer to
  the hardware texture data can be shared among many structures.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define P3DC_TXTR_BASE		1	/* A decal texture */
#define P3DC_TXTR_LIGHTMAP	2	/* A light map texture */
#define P3DC_TXTR_DETAIL	4	/* A detail texture */

#define P3DC_TXTR_WRAP		8	/* wrap mode */
#define	P3DC_TXTR_LOADED	16	/* Texture loaded state. */

typedef struct __p3dc_texture {
	void		*data;	/* Texture data (Internal format). */
	int			mode;	/* Texture type (Decal, light, detail and Wrap or Nowrap) */
} p3dc_TDATA;

typedef struct __p3dc_face_texture {
	p3dc_TDATA	*tex;	  /* The texture to use. Can not be null! */
	p3dc_LIST	vrtx;	  /* A list of u,v pairs */
} p3dc_TXTR;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	V E R T E X   D E F I N I T I O N

  Vertices are the bonds that hold together objects in P3DC. A vertex 
  contains a POINT3 which holds its x, y, and z co-ordindates. It also
  contains a transform id (xid) and a POINT4. The latter two components
  are used when the vertex is transformed. In order to speed up the
  library, when a vertex is transformed its transformed value is stored
  in the POINT4 and the identifier of the transformation that made that
  transformation is stored in xid. The next time through, if the xid in
  the vertex matches the id of the transformation matrix then the vertex
  is NOT retransformed. (saves doing the same thing twice) The catch is
  that you *MUST* change the id of a transformation matrix whenever its
  value changes!
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

typedef struct p3dc_vertex {
	p3dc_PNT3		v;		/* position in 3 space for this vertex */
	/* ---- everything below this point is derived in the library ---- */
	unsigned short	xid;	/* Transform ID (identifies the transform used) */
	p3dc_PNT4		tv;		/* Transformed vertex values */

	unsigned short	cid;	/* Clip ID */
	unsigned short	ccode;	/* Clip code */
} p3dc_VRTX;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	L I G H T   D E F I N I T I O N

  Lights are used to illuminate the scene. They currently come in two
  types, spot and ambient. Spotlights shine a light of a particular
  color from a single point.
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
typedef enum light_type {
	P3DC_LIGHT_SPOT,
	P3DC_LIGHT_AMBIENT,
	P3DC_LIGHT_DIRECTIONAL,
	P3DC_LIGHT_OMNI,
} p3dc_LIGHT_TYPE;


typedef struct __p3dc_light {
	p3dc_VRTX		origin;	/* Where the light is located (world co-ords) */
	p3dc_PNT3		color;	/* The light's color */	
	p3dc_PNT3		dir;	/* Where the light is pointed */
	p3dc_PNT3		depth;	/* For spotlights */
	p3dc_FLOAT		scale;	/* For spotlights/ambient */
	p3dc_LIGHT_TYPE	type;	/* type of light used */
} p3dc_LIGHT;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	P O I N T   D E F I N I T I O N

  The point is the simplest drawing primitive, it consists of a color,
  a vertex, and some lighting information. The Light Id (lid) is the
  same as the transform ID of the last camera that has transformed this
  point. This is because points are re-lit each time the camera changes.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

typedef struct __p3dc_point {
	p3dc_VRTX		*p0;	/* The points location in space */
	p3dc_CLR		*color;	/* The points color */
	short			lid;	/* Transform ID for lighting */
	p3dc_PNT4		light;	/* Lighting attributes */
} p3dc_POINT;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	L I N E   D E F I N I T I O N

  Next up are lines. They aren't much more complicated than points,
  simply adding an additional vertex to the point structure. I was
  tempted to make it literally an extension of the point structure 
  but there wasn't any real benefit in doing that.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

typedef struct __p3dc_line {
	p3dc_VRTX	*p1,	/* The line segment's endpoints */
				*p2;
	p3dc_CLR	*color;	/* The points color */
	short		cid;	/* Clip identifier */
	int			ccode;	/* Clip code for the line */
	p3dc_PNT4	c1,		/* The clipped versions of the vertices */ 
				c2;		/* Are cached here. */
	short		lid;	/* Transform ID for lighting */
	p3dc_PNT4	light;	/* Lighting attributes */
} p3dc_LINE;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	P O L Y G O N   D E F I N I T I O N

  These are PRIMITIVE polygons, that means they aren't Gourard shaded
  and they aren't textured (that is for faces below). They are
  constrained to be planar (all vertices in them are on the same plane)
  and they are supposed to be convex. Further, the vertices should be
  listed in counter clockwise order if you want the normals to come out
  correctly. 

  In addition to the other stuff, the polygons keep the results of being
  clipped around so that when the camera hasn't changed they don't need
  to be re-clipped. This speeds things up a lot when you have a static
  scene with a fixed camera and a few models moving about in it.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

typedef struct __p3dc_poly {
	p3dc_LIST		pn;	/* Polygon vertex list */
	p3dc_PNT4		light;	/* Base lighting effect on the face */
	p3dc_PNT3		norm;	/* Face normal vector */
	p3dc_CLR		*color;	/* Base color of the face */
	short			xid;	/* Transformation identifier */
	short			lid;	/* Lighting identifier */
	p3dc_LIST		*clip;	/* Clipped version. */
	short			ccode;	/* the last clip code */
	short			cid;	/* Clip ID == XID of camera */
} p3dc_POLY;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	F A C E   D E F I N I T I O N

  If vertices are the bonds, then faces are the lumber that makes up
  objects in P3DC. Technically a misnomer since faces with only one
  vertex are actually points, and faces with two vertices are actually
  line segments, however they are all treated the same by the library.

  A couple of invariants have to be true with faces, for one the
  vertices are specified in counter-clockwise (CCW) order. By doing
  this their front normal points "out". This matches the way that Glide
  determines how to do backface culling. 

  Next the value 'nV' (number of vertices) is used to size all of the
  lists pointed to by the structure. There are three of interest,
	- The vertex list (this one is pretty obvious)
	- The normals list (this is an array of normals for the _vertices_)
	- The texture co-ordinate array. (this is subtle since the texture
	  pointer points off at another structure.)

  Finally, the face, when it is a polygon, must be both planar AND
  convex. (otherwise it won't clip right).

  The node lists were designed so that the vertices and normals could
  be shared amongst different faces in the same model.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Names of the data list lists */
#define P3DC_FACE_VERTEX	0	/* vertices of the face */
#define P3DC_FACE_LIGHTS	1	/* Light values for the vertices */
#define P3DC_FACE_BASE		2	/* A base texture */
#define P3DC_FACE_LIGHTMAP	3	/* A light map texture */
#define P3DC_FACE_DETAIL	4	/* A detail texture to use when close */

typedef struct __p3dc_face {
	p3dc_POLY		poly;		/* Base polygon for the face */
	short			lid;		/* Lighting identifier */
	p3dc_LIST		*lights;	/* Light values for G shading. */
	p3dc_LIST		vnorm;		/* Vertex normals (for Gourard Shading) */
	p3dc_LIST		txtr;		/* Pointer to linked list of textures */
	p3dc_LIST		*(ancil[5]);	/* List of lists, ancillary data (static) */
	p3dc_LIST		*(clip[5]);	/* Clipped version of the face's ancillary data (dynamic) */
	short			ccode;		/* the last clip code computed */
	short			cid;		/* clip identifier */
	short			xid;
} p3dc_FACE;
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	M O D E L   D E F I N I T I O N 

  Models are the movable objects in the world. They are composed of a 
  list of parts and vertices that are transformed from their own 'model'
  space into world space, and from there into camera space. Construction
  of the model structures is done by the model parser. The parser reads
  a model specification and constructs the appropriate model.
*/
#define P3DC_STANDARD_MODELS "stdmodels.3dc"
#define P3DC_USER_MODELS "usrmodels.3dc"

typedef struct __p3dc_model {
	p3dc_LIST	parts;		/* Parts used by the components      */
	p3dc_LIST	components;	/* A linked list of things in this model */
	char		*name;		/* The model's name */
	p3dc_LIST	cache;
	p3dc_CLR		color;
	p3dc_XFRM	O;				/* Model to world transform */
	p3dc_XFRM	V;				/* Model to view transform */
	unsigned	xid;			/* Camera transform ID */
	p3dc_PNT3	origin;
	p3dc_PNT3	eulera;			/* x = pitch, y = yaw, z = roll */
	p3dc_PNT3	loc;			/* The model's location in three space */
	int			flags;			/* Model flags */
	p3dc_VRTX	corners[8];
	p3dc_LIST	box;
} p3dc_MODEL;


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	C A M E R A   D E F I N I T I O N

  Cameras in P3DC are your eyes on the world. If you want to see what is
  going on, you have to have a camera to look at it. The visible
  manifestation of a camera is a rectangular piece of the screen that is
  looking at some piece of the 3D world. 
  
  The cameras are used for two things:
		Defining the clipping rectangle.
		Defining a view of view/perspective transformation.

  They are specified by a position in the world, a direction vector for
  where they are "looking" and a rotation angle. By default their "up"
  vector is the Y axis.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Control definitions for cameras */
#define P3DC_CAMERA_ERR		0
#define P3DC_CAMERA_DIR		1
#define P3DC_CAMERA_LOOK	2
#define P3DC_CAMERA_ORIGIN	3
#define P3DC_CAMERA_TWIST	4
#define P3DC_CAMERA_NEAR	5
#define P3DC_CAMERA_FAR		6
#define P3DC_CAMERA_UP		7
#define P3DC_CAMERA_FOV		8
#define P3DC_CAMERA_ROLL	9
#define P3DC_CAMERA_PITCH	10
#define P3DC_CAMERA_YAW		11
#define P3DC_CAMERA_SCREEN	12
#define P3DC_CAMERA_VIEW	13


typedef struct p3dc_viewport {
	p3dc_FLOAT	scale_x,	/* Screen scaling factors for X ... */
			scale_y,	/*        ... and Y */
			center_x,	/* Position of the viewport for X ... */
			center_y;	/*        ... and Y */
} p3dc_VIEW;

typedef struct p3dc_camera {
	p3dc_PNT3	origin;		/* Camera origin. */
	p3dc_PNT3	look;		/* The point we are looking 'at' (same as W2V column 3)*/
	p3dc_PNT3	up;			/* The "up" vector. (same as W2V column 2)*/
	p3dc_FLOAT	fov;		/* Field of view (35 degrees is the default) */
	p3dc_FLOAT	twist;		/* "twist" around the Z axis */
	p3dc_FLOAT	far_p;		/* A 'far' clipping plane for Z */
	p3dc_FLOAT	near_p;		/* THe 'near' clipping plane for Z */
	p3dc_XFRM	W2V;		/* World => View Transform */
	p3dc_XFRM	V2S;		/* View => Screen Transform (includes perspective xfrm) */
	p3dc_XFRM	view_xfrm;	/* View transform (w/Perspective applied) */
	p3dc_VIEW	vp;			/* The viewport for this camera */
} p3dc_CMRA;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	C O N S T A N T S   A N D   I N V A R I A N T S

  This section defines constants used by the P3DC libraries
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* The magic number in graphics ... */
#define PI			(3.14159265358979323846f)

/* Conversion constant between degrees and radians */
#define	DTR			(PI/180.0f)	
#define D2R(d)		((d) * 0.01745329f)
#define R2D(r)		((r) * 57.29577951f)

/* Minimum value in single precision */
#define P3DC_SIGMA	(.0001f)

/* Equivalent to zero boolean for floating point numbers */
#define EQZ(x)		(fabs((x)) < P3DC_SIGMA)

#define P3DC_DISPLAY_OFF			0
#define P3DC_DISPLAY_ON			1
#define P3DC_DISPLAY_MOVE		2
#define P3DC_DISPLAY_RESIZE		3

#define P3DC_CTRL_GETRESOLUTION	0	/* int *w, int *h */
#define P3DC_CTRL_SETRESOLUTION	1	/* int w, int h */
#define P3DC_CTRL_DISPLAY		2	/* int on/off/move/resize */
#define P3DC_CTRL_FOGCOLOR		3	/* Set the fogging color to this. */
#define P3DC_CTRL_DEPTHBUFFER	4	/* Set the depth buffer on or off */
#define P3DC_CTRL_GSHADING		5	/* Enable/disable Gourard Shading */
#define P3DC_CTRL_TRANSPARENT	6	/* Enable/disable Gourard Shading */
#define P3DC_CTRL_OPAQUE			7	/* Enable/disable Gourard Shading */
#define P3DC_CTRL_LIGHTMAP		8	/* Enable/disable light map */
#define P3DC_CTRL_DEPTHBIAS		9	/* Enable/disable light map */

/* Drawing controls, note that '0' is everything on ... */
#define P3DC_DRAW_NOCLIP	1<<0	/* Clip to camera before Drawing */
#define P3DC_DRAW_NOSHADE	1<<1	/* Draw with shading in effect */
#define P3DC_DRAW_NOFILL	1<<2	/* Draw with polygons filled */
#define P3DC_DRAW_NOTEXTURE	1<<3	/* Draw with textures enabled */
#define P3DC_DRAW_BBOX		1<<4	/* Draw model bounding boxes */
#define P3DC_DRAW_WIRE		1<<5	/* Draw model using wireframe */

#define P3DC_DRAW_DEFAULT	P3DC_DRAW_CLIPPED | \
							P3DC_DRAW_SHADED | \
							P3DC_DRAW_CLIPPED | \
							P3DC_DRAW_TEXTURED

#define P3DC_TEXTURE_NONE	0

#define CLIP_NONE	0	/* Both points are off screen			*/
#define CLIP_A_ONLY	1	/* A is on the edge (degenerate case)	*/
#define CLIP_A_X	2	/* B is modified to be on-screen		*/
#define CLIP_A_B	3	/* Both A and B are visible (no change) */
#define CLIP_X_B	4	/* A is modified, B is on-screen.		*/
#define CLIP_B_ONLY	5	/* B is on the edge (degenerate case)	*/
#define CLIP_X_X	6	/* Both A and B are outside the view	*/
#define CLIP_ERROR	7	/* Point wasn't transformed				*/

/* 
 * Plane definition:
 */
#define NEG_Z_PLANE	0 /* -Z plane */
#define NEG_Y_PLANE	1 /* -Y plane */
#define POS_Y_PLANE	2 /* +Y plane */
#define NEG_X_PLANE	3 /* -X plane */
#define POS_X_PLANE	4 /* +X plane */
#define POS_Z_PLANE	5 /* +Z plane */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	P R O T O T Y P E   D E F I N I T I O N S

  This section contains prototypes for all of the P3DC user visible library
  functions.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/******************************************
 *               Transform API            *
 ******************************************/

/* Initialize/Allocate a matrix to the identity matrix */
extern p3dc_XFRM *p3dc_identity_xfrm(p3dc_XFRM *);

/* Frees a transform (and nullifies its ptr) that is no longer needed */
extern void p3dc_free_xfrm(p3dc_XFRM **);

/* Multiply A by B and return it in res (or alloc if res == NULL) */
extern p3dc_XFRM *p3dc_mult_xfrm(p3dc_XFRM *res, p3dc_XFRM *A, p3dc_XFRM *B);

/* Set a translation in a transform */
extern p3dc_XFRM *p3dc_locate_xfrm(p3dc_XFRM *res, p3dc_FLOAT x, p3dc_FLOAT y, p3dc_FLOAT z);

/* Create a translation tranform from a vector/scalar pair */
extern p3dc_XFRM *p3dc_move_xfrm(p3dc_XFRM *res, p3dc_PNT3 *dir, p3dc_FLOAT dist);

/* Create a transform with non-uniform scaling constants */
extern p3dc_XFRM *p3dc_scaleXYZ_xfrm(p3dc_XFRM *res, p3dc_FLOAT x, p3dc_FLOAT y, p3dc_FLOAT z);

/* Create a transform with uniform scaling */
extern p3dc_XFRM *p3dc_scale_xfrm(p3dc_XFRM *res, p3dc_FLOAT scale);

/* Apply a rotation of 'deg' degrees about the Z axis to a matrix */
extern p3dc_XFRM *p3dc_zrot_xfrm(p3dc_XFRM *A, p3dc_FLOAT deg);

/* Apply a rotation of 'deg' degrees about the Y axis of a matrix */
extern p3dc_XFRM *p3dc_yrot_xfrm(p3dc_XFRM *A, p3dc_FLOAT deg);

/* Apply a rotation of 'deg' degrees about the X axis of a matrix */
extern p3dc_XFRM *p3dc_xrot_xfrm(p3dc_XFRM *A, p3dc_FLOAT deg);

/* Create an inverse transform given a point and a direction */
extern p3dc_XFRM *p3dc_orientation_xfrm(p3dc_XFRM *r, p3dc_PNT3 *dir, p3dc_PNT3 *orig, p3dc_PNT3 *up, float roll);
extern void p3dc_dump_xfrm(p3dc_XFRM *, char *title, char *header, char *left, char *tail);

/*************************************************
 *         Vertex API                            *
 *************************************************/

/* Allocate a new vertex structure */
extern p3dc_VRTX *p3dc_new_vrtx(p3dc_FLOAT x, p3dc_FLOAT y, p3dc_FLOAT z);

/* Free a previously allocated vertex for re-use */
extern void p3dc_free_vrtx(p3dc_VRTX **v);

/* Transform a vertex using the passed in Transformation matrix */
extern void p3dc_xfrm_vrtx(p3dc_VRTX *r, p3dc_XFRM *t, p3dc_VRTX *src);

/* Compute the vector product of two vertices at the origin */
extern p3dc_VRTX *p3dc_cross_vrtx(p3dc_VRTX *r, p3dc_VRTX *a, p3dc_VRTX *b);

/* Compute the vector product of two vertices not at the origin */
extern p3dc_VRTX *p3dc_cross3_vrtx(p3dc_VRTX *r, p3dc_VRTX *origin, p3dc_VRTX *a, p3dc_VRTX *b);

/* Subtract two vectors (computes the vector a->b  between them) */
extern p3dc_VRTX *p3dc_sub_vrtx(p3dc_VRTX *r, p3dc_VRTX *a, p3dc_VRTX *b);

/* Compute the inner product (dot product) of two vectors at the origin */
extern p3dc_FLOAT p3dc_dot_vrtx(p3dc_VRTX *a, p3dc_VRTX *b);

/* Compute the inner product (dot product) of two vectors at an arbitrary origin */
extern p3dc_FLOAT p3dc_dot3_vrtx(p3dc_VRTX *o, p3dc_VRTX *a, p3dc_VRTX *b);

/* Normalize the passed vector to have unit length */
extern p3dc_VRTX *p3dc_unit_vrtx(p3dc_VRTX *r, p3dc_VRTX *p); 

/* Compute a plane normal given three CCW points on the plane */
extern p3dc_VRTX *p3dc_normal_vrtx(p3dc_VRTX *r, p3dc_VRTX *a, p3dc_VRTX *b, p3dc_VRTX *c);

/* Scale a vertex (scalar multiply) by a given amount */
extern p3dc_VRTX *p3dc_scale_vrtx(p3dc_VRTX *r, p3dc_VRTX *v, p3dc_FLOAT sc); 

/* Add two vertices to get a third */
extern p3dc_VRTX *p3dc_add_vrtx(p3dc_VRTX *r, p3dc_VRTX *a, p3dc_VRTX *b); 

/*************************************************
 *         Clipping API                          *
 *************************************************/

/* Clip a point in world base into the view port represented by a camera */
extern int p3dc_clip_point(p3dc_POINT *);

/* Clip a line segment in world space into the camera view port */
extern int p3dc_clip_line(p3dc_LINE *line, float *delta1, float *delta2);

/* Clip a set of lists (polygons, lights, textures etc) */
extern int p3dc_clip_lists(p3dc_LIST *(src[]), p3dc_LIST *(dst[]));

/* Free a list of clipped vertices in a polygon */
extern void p3dc_clip_free_cliplist(p3dc_LIST *);

/*************************************************
 *        Memory Pool Allocator API              *
 *************************************************/

/* Allocate a listed item */
extern void *p3dc_new_type(p3dc_TYPE t);
extern void p3dc_reset_memstats(void);
extern void p3dc_memstats(int reset);

/* Returns this type to a pool of free structures. */
extern void p3dc_free_type(void *p);

/* If this is allocated by p3dc_new_type above, this will return its type */
extern p3dc_TYPE p3dc_get_type(void *p);

/*********************************************
 *              Logging API                  *
 *********************************************/
 
/* Open the P3DC log, optionally open it into the file name specified */
extern void p3dc_log_open(char *);

/* Flush pending writes to the log ... */
extern void p3dc_log_flush(void);

/* Close the log. */
extern void p3dc_log_close(void);

/* Write to the log, this works just like printf() does. */
extern void p3dc_log(char *, ...);

/*************************************************
 *          Rendering Functions                  *
 *************************************************/
 
/* Drawing functions for G3d */
extern void     p3dc_draw_point(p3dc_VIEW *vp, p3dc_XFRM *view, int flags, p3dc_POINT *pnt);
extern void      p3dc_draw_line(p3dc_VIEW *vp, p3dc_XFRM *view, int flags, p3dc_LINE *line);
extern void   p3dc_draw_polygon(p3dc_VIEW *vp, p3dc_XFRM *view, int flags, p3dc_POLY *poly);
extern void      p3dc_draw_face(p3dc_VIEW *vp, p3dc_XFRM *view, p3dc_XFRM *world, int flags, p3dc_FACE *face);
extern void p3dc_draw_primitive(p3dc_VIEW *vp, p3dc_XFRM *view, int flags, p3dc_NODE *prim);
extern void      p3dc_draw_list(p3dc_VIEW *vp, p3dc_XFRM *view, int flags, p3dc_LIST *prim_list);
extern void     p3dc_draw_model(p3dc_VIEW *vp, p3dc_XFRM *view, int flags, p3dc_MODEL *m);
extern void      p3dc_draw_axes(p3dc_VIEW *vp, p3dc_XFRM *view, int flags, p3dc_FLOAT scale); 

/*************************************************
 *               Camera functions                *
 *************************************************/

/* Create a new camera (view port) with which to view things */
extern p3dc_CMRA *p3dc_new_camera(p3dc_FLOAT fov, p3dc_FLOAT x, p3dc_FLOAT y, p3dc_FLOAT z);

/* Initialize a camera structure */
extern void p3dc_init_camera(p3dc_CMRA *cam, p3dc_FLOAT fov, p3dc_FLOAT x, p3dc_FLOAT y, p3dc_FLOAT z);

/* Set one or more attributes of the camera */
extern int p3dc_set_camera(p3dc_CMRA *, int parameter, ...);

/* Change the field of view (zoom) the camera */
extern void p3dc_set_camera_fov(p3dc_CMRA *cam, p3dc_FLOAT zoom);

/* Change the position of the camera */
extern void p3dc_set_camera_origin(p3dc_CMRA *cam, p3dc_FLOAT x, p3dc_FLOAT y, p3dc_FLOAT z);

/* Change what the camera is looking at (target it) */
extern void p3dc_set_camera_target(p3dc_CMRA *cam, p3dc_FLOAT x, p3dc_FLOAT y, p3dc_FLOAT z, 
								   p3dc_PNT3 *up, p3dc_FLOAT twist);

/* Set a new direction vector for the camera */
extern void p3dc_set_camera_dir(p3dc_CMRA *cam, p3dc_PNT3 *dir, p3dc_PNT3 *up, p3dc_FLOAT twist);

/* Change the size of the viewport dedicated to this camera */
extern void p3dc_set_camera_window(p3dc_CMRA *cam, int x, int y);

/* Create a new frame for this camera to display within */
extern void p3dc_set_camera_frame(p3dc_CMRA *cam, int lx, int ly, int hx, int hy);

/* Change the far and near clip planes for the camera */
#define p3dc_set_camera_far(cm, fp)  p3dc_set_camera((cm), P3DC_CAMERA_FAR, fp, 0)
#define p3dc_set_camera_near(cm, np) p3dc_set_camera((cm), P3DC_CAMERA_NEAR, np, 0)

/* Set the world to view transformation */
extern void p3dc_set_camera_view(p3dc_CMRA *cam, p3dc_XFRM *world_to_view);

/* Set the view to screen transformation */
extern void p3dc_set_camera_screen(p3dc_CMRA *cam, p3dc_XFRM *view_to_screen);

/* Roll the camera (positive is clockwise ... */
extern void p3dc_set_camera_roll(p3dc_CMRA *cam, p3dc_FLOAT deg);
extern void p3dc_set_camera_pitch(p3dc_CMRA *cam, p3dc_FLOAT deg);
extern void p3dc_set_camera_yaw(p3dc_CMRA *cam, p3dc_FLOAT deg);


/************************************************
 *           Utility Functions                  *
 ************************************************/
extern void p3dc_adjust_color(p3dc_CLR *src, p3dc_FLOAT shade, p3dc_CLR *dst);
extern void p3dc_adjust_alpha(p3dc_CLR *src, p3dc_FLOAT shade, p3dc_CLR *dst);
extern void p3dc_apply_light(p3dc_CLR *src, p3dc_PNT4 *light, p3dc_CLR *dst);

/* Normalize (make unit length) a 3POINT vector, return true if zero length */
extern int p3dc_normalize_pnt3(p3dc_PNT3 *);

/************************************************
 *         Lighting API                         *
 ************************************************/

/* Adjust lighting parameters for a point */
extern void p3dc_light_point(p3dc_PNT3 *, p3dc_POINT *p);

/* Adjust lighting parameters for a line */
extern void p3dc_light_line(p3dc_PNT3 *, p3dc_LINE *);

/* Adjust the lighting parameters for a polygon */
extern void p3dc_light_polygon(p3dc_POLY *pn);

/* Adjust the lighting parameters for a face */
extern p3dc_LIST *p3dc_light_face(p3dc_FACE *f);

/* Add a light to the world */
extern void p3dc_light_add(p3dc_LIGHT *l);

/* Initialize the lights subsystem to have just the sun */
extern void p3dc_light_init();

/* Reset the lights subsystem, ie lights out! */
extern void p3dc_light_reset();

/***************************************************
 *                 Housekeeping                    *
 ***************************************************/

/* Initialize the library */
extern int p3dc_open(void);

/* Close the library */
extern void p3dc_close(void);

/* Prepare the frame for drawing */
extern void p3dc_prepare_frame(p3dc_CLR *);

/* Display the current frame */
extern void p3dc_flip_frame(int);

/* Set the color of the display background */
extern void p3dc_clear_viewport(p3dc_CLR *c, p3dc_VIEW *vp);

/* Clear a portion of the screen */
extern void p3dc_clear_region(p3dc_CLR *clr, p3dc_FLOAT lx, p3dc_FLOAT ly, p3dc_FLOAT ux, p3dc_FLOAT uy);

/* Copy the framebuffer to a Portable Network Graphics File */
extern int p3dc_snapshot(char *file, char *author, p3dc_CLR *chroma);

extern int p3dc_display_ctrl(int func, ...);

/***************************************************
 *            List Management                      *
 ***************************************************/

extern int p3dc_add_node(p3dc_LIST *, p3dc_NODE *, p3dc_NODE_POSITION, ... );
extern p3dc_NODE *p3dc_remove_node(p3dc_LIST *, p3dc_NODE_LOCATION, ... );
extern p3dc_NODE *p3dc_get_node(p3dc_LIST *, p3dc_NODE_LOCATION, ... );

/* Allocate a node and give it a particular payload */
extern p3dc_NODE *p3dc_new_node(void *, char *name, int flags);

/* Initialize a list */
extern void p3dc_init_list(p3dc_LIST *, p3dc_TYPE, p3dc_LIST_TYPE);
extern p3dc_LIST *p3dc_new_list(p3dc_TYPE, p3dc_LIST_TYPE);

/* Free a node that was on a list */
extern void p3dc_free_node(p3dc_NODE *);

/* Free an entire list */
extern void p3dc_free_list(p3dc_LIST *);

extern void p3dc_rbprint_list(p3dc_LIST *);

/* some syntactic sugar */
#define FIRST_NODE(list) \
		p3dc_get_node((list), P3DC_NODE_FIRST)

#define NEXT_NODE(list)	 \
		p3dc_get_node((list), P3DC_NODE_NEXT)

#define APPEND_NODE(list, node) \
	 	p3dc_add_node((list), (node), P3DC_LIST_TAIL)

/******************************************************
 *       Primitive alloc/free                         *
 ******************************************************/

extern p3dc_CLR *p3dc_new_color(p3dc_CLR *);
#define p3dc_free_color(c) { p3dc_free_type(c); }

/* Allocate and free p3dc_LINE structures */
extern p3dc_LINE *p3dc_new_line(p3dc_CLR *, p3dc_PNT3 *, p3dc_PNT3 *);
extern void p3dc_free_line(p3dc_LINE *);

/* Allocate and free p3dc_POINT structures */
extern p3dc_POINT *p3dc_new_point(p3dc_CLR *, p3dc_PNT3 *);
extern void p3dc_free_point(p3dc_POINT *);

/* Allocate and free p3dc_POLY structures */
extern p3dc_POLY *p3dc_new_polygon(p3dc_CLR *, int nVrts, p3dc_PNT3 vrtx[]);
extern void p3dc_free_polygon(p3dc_POLY *);

/******************************************************
 *            Model API                               *
 ******************************************************/

/* Dump a model (decompile it) to the log file */
extern void p3dc_dump_model(p3dc_MODEL *m);

/* Position a model in the world. */
extern void p3dc_locate_model(p3dc_MODEL *m, p3dc_FLOAT x, p3dc_FLOAT y, p3dc_FLOAT z);

/* Allocate a new model */
extern p3dc_MODEL *p3dc_new_model(char *name, p3dc_FLOAT sx, p3dc_FLOAT sy, p3dc_FLOAT sz);

/* Point the model at a given point in 3D space */
extern void p3dc_target_model(p3dc_MODEL *m, p3dc_FLOAT x, p3dc_FLOAT y, p3dc_FLOAT z);
extern void p3dc_point_model(p3dc_MODEL *m, p3dc_PNT3 *dir, p3dc_PNT3 *up, p3dc_FLOAT twist);

/* Change its pitch (rotate about X axis) */
extern void p3dc_pitch_model(p3dc_MODEL *m, p3dc_FLOAT delta);

/* Change its roll (rotate about Z axis) */
extern void p3dc_roll_model(p3dc_MODEL *m, p3dc_FLOAT delta);

/* Change its yaw (rotate about Y axis) */
extern void p3dc_yaw_model(p3dc_MODEL *m, p3dc_FLOAT delta);

/* Reset model to its internal euler angle orientation */
extern void p3dc_reset_model(p3dc_MODEL *m);

/* Adjust model's location along the model's Z axis */
extern void p3dc_fwd_model(p3dc_MODEL *m, p3dc_FLOAT delta);

/* Adjust model's location backward along the model's Z axis */
extern void p3dc_bkwd_model(p3dc_MODEL *m, p3dc_FLOAT delta);

/* Change a color in the current model */
extern int p3dc_model_set_color(p3dc_MODEL *, char *name, p3dc_CLR *);
extern int p3dc_model_set_all_colors(p3dc_MODEL *, p3dc_CLR *);

/* Change the texture in a model */
extern int p3dc_model_set_texture(p3dc_MODEL *, char *name, char *texture_name);
extern int p3dc_model_set_all_textures(p3dc_MODEL *, char *texture_name);

/**********************************************************
 *           Texture Mangement API                        *
 **********************************************************/

/* allocate a new texture */
extern p3dc_TDATA *p3dc_new_texture_data(int mode);
extern int p3dc_load_texture(char *name, p3dc_TDATA *td); 

/* free a texture that is no longer needed */
extern void p3dc_free_texture_data(p3dc_TDATA *txt);
extern p3dc_TXTR *p3dc_new_face_texture(p3dc_TDATA *td);


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	G A L   L A Y E R   P R O T O T Y P E   D E F I N I T I O N S

  This section contains prototypes for the graphics abstraction layer.
  In the case of glide, this is where glide plugs into P3DC
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
extern void * __hw_load_texture(char *, int, int);
extern void __hw_unload_texture(void *);
extern void	__hw_enable_texture(void *);
extern void	__hw_disable_texture(void);
// extern void __hw_texture_stats(p3dc_TextureStats *s);
extern void __hw_texture_mode(int);

extern int __hw_init(void);
extern void __hw_shutdown(void);
extern void __hw_prepare_frame(p3dc_CLR *);
extern void __hw_show_frame(int);
extern void __hw_effect_fog(p3dc_CLR *);
extern void __hw_draw_point(p3dc_VIEW *, p3dc_POINT *);
extern void __hw_draw_line(p3dc_VIEW *, p3dc_LINE *);
extern void __hw_draw_polygon(p3dc_VIEW *, p3dc_POLY *, p3dc_LIST *);
extern void __hw_draw_face(p3dc_VIEW *e, p3dc_FACE *ff, p3dc_LIST *(list[]), void *, int); 
#ifndef __MSC__
extern int __hw_control();
#else
extern int __hw_control(int, ...);
#endif
extern void __hw_clear_region(p3dc_CLR *cl, float x, float y, float w, float h);
extern int __hw_get_row(int, unsigned char []);

#define p3dc_cross_pnt3(p, q, result) { \
	(result)->x = (p)->y*(q)->z - (p)->z*(q)->y; \
	(result)->y = (p)->z*(q)->x - (p)->x*(q)->z; \
    (result)->z = (p)->x*(q)->y - (p)->y*(q)->x; } 

#define p3dc_dot_pnt3(a, b) ((a)->x * (b)->x + \
							(a)->y * (b)->y +  \
							(a)->z * (b)->z )

#define p3dc_diff_pnt3(a, b, result) {(result)->x = (b)->x - (a)->x; \
							 (result)->y = (b)->y - (a)->y; \
							 (result)->z = (b)->z - (a)->z; }

