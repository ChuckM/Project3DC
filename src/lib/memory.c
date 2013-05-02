/*
 * memory.c - Implementation of the fast memory allocator routines
 * $Id: memory.c,v 1.7 1999-10-31 11:23:59-08 cmcmanis Exp cmcmanis $
 *
 * One of the things I do to try to make this engine reasonably fast
 * is to give it a reasonably fast memory allocation architecture. The
 * architecture is simple. (which always helps reliability and correctness)
 * 
 * For each basic type in P3DC that I allocate and deallocate, I provide
 * a "pool" of previously allocated structures from which to choose. When
 * an item is freed, it is put on the pool, when an item is allocated it
 * is removed from the pool. If the pool is empty when an allocation request
 * arrives, a new structure is malloc'd. 
 *
 * What happens in practice is that the first few frames allocate a bunch
 * of vertices etc, and then free them before rendering the next frame. On
 * the second through nth frame the allocation is very fast (two pointer
 * stores and you're done.) Thus there is a burst of malloc activity and
 * then the memory size stabilizes. This has the negative effect that you
 * run at the biggest possible size needed. If this becomes a problem I'll
 * add an 'aging' feature that frees things that have not been re-used
 * in a while.
 *
 * Change History:
 *	$Log: memory.c,v $
 *	Revision 1.7  1999-10-31 11:23:59-08  cmcmanis
 *	Camera rewrite is nearly complete. This seems like a logical
 *	milestone
 *
 *	Revision 1.6  1999/09/18 21:21:36  cmcmanis
 *	Added NODE_SIZE macro for portability across platforms.
 *
 *	Revision 1.5  1999/09/12 06:31:08  cmcmanis
 *	Rewrote clipping api (again?)
 *
 *	Revision 1.3  1999/09/09 02:39:55  cmcmanis
 *	Added memory statistics
 *
 *	Revision 1.2  1999/09/02 03:22:39  cmcmanis
 *	The great LIST rewrite, lists are much cleaner now
 *
 *	Revision 1.1  1999/08/18 01:33:48  cmcmanis
 *	Initial revision
 *
 *	Revision 1.1  1999/08/18 01:06:16  cmcmanis
 *	Initial revision
 *
 *	Revision 1.7  1999/07/25 09:11:54  cmcmanis
 *	The great renaming from g3d/G3D -> p3dc/P3DC
 *
 *	Revision 1.6  1998/11/16 03:40:57  cmcmanis
 *	Updated clipping
 *
 *	Revision 1.6  1998/11/08 05:20:30  cmcmanis
 *	Outdated (renamed to 'H:\P3DC-RCS\src\lib\memory.c,v').
 *
 *	Revision 1.5  1998/10/10 07:00:39  cmcmanis
 *	Model parsing is written (not fully debugged) and the texture structures
 *	are closer to their final form. By the next check in we should have a
 *	few simple models up and running with textured models to follow.
 *
 *	Revision 1.4  1998/09/23 08:48:55  cmcmanis
 *	Clipping works! Polygon and line clipping is pretty robust at this point
 *	still looking for corner cases but I believe all of the code is correct at
 *	this point.
 *
 *	Revision 1.3  1998/09/20 08:10:56  cmcmanis
 *	Major rewrite, added list stuff, and primitives. Almost completely
 *	back to where version 4 was at this point. This version is ALPHA0_2
 *
 *	Revision 1.2  1998/09/18 08:34:41  cmcmanis
 *	This update has new clipping code for polygons.
 *	In particular the nodes are not needlessly copied.
 *
 *	Adds the new list capability in util.c/memory.c that
 *	uses a p3dc_NLST (type P3DC_LIST) as a simple
 *	linked list.
 *
 *	Revision 1.1  1998/09/04 05:46:59  cmcmanis
 *	Initial revision
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
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <p3dc.h>

/*
 * Define NODE SIZE as the next long word aligned address
 * after a NODE.
 */
#define NODE_SIZE ((sizeof(p3dc_NODE)+3) & ~3)

static p3dc_LIST notype_pool = { NULL, NULL, P3DC_NOTYPE, P3DC_LIST_LINKED};
static p3dc_LIST float_pool  = { NULL, NULL, P3DC_FLOAT, P3DC_LIST_LINKED};
static p3dc_LIST pnt2_pool   = { NULL, NULL,  P3DC_PNT2, P3DC_LIST_LINKED};
static p3dc_LIST pnt3_pool   = { NULL, NULL,  P3DC_PNT3, P3DC_LIST_LINKED};
static p3dc_LIST pnt4_pool   = { NULL, NULL,  P3DC_PNT4, P3DC_LIST_LINKED};
static p3dc_LIST xfrm_pool   = { NULL, NULL,  P3DC_XFRM, P3DC_LIST_LINKED};
static p3dc_LIST tdta_pool   = { NULL, NULL,  P3DC_TDATA, P3DC_LIST_LINKED};
static p3dc_LIST txtr_pool   = { NULL, NULL,  P3DC_TXTR, P3DC_LIST_LINKED};
static p3dc_LIST nlst_pool   = { NULL, NULL,  P3DC_NODE, P3DC_LIST_LINKED};
static p3dc_LIST clr_pool    = { NULL, NULL,   P3DC_CLR, P3DC_LIST_LINKED};
static p3dc_LIST vrtx_pool   = { NULL, NULL,  P3DC_VRTX, P3DC_LIST_LINKED};
static p3dc_LIST face_pool   = { NULL, NULL,  P3DC_FACE, P3DC_LIST_LINKED};
static p3dc_LIST cmra_pool   = { NULL, NULL,  P3DC_CMRA, P3DC_LIST_LINKED};
static p3dc_LIST type_pool   = { NULL, NULL,  P3DC_TYPE, P3DC_LIST_LINKED};
static p3dc_LIST lite_pool   = { NULL, NULL,  P3DC_LIGHT, P3DC_LIST_LINKED};
static p3dc_LIST point_pool  = { NULL, NULL, P3DC_POINT, P3DC_LIST_LINKED};
static p3dc_LIST line_pool   = { NULL, NULL,  P3DC_LINE, P3DC_LIST_LINKED};
static p3dc_LIST poly_pool   = { NULL, NULL,  P3DC_POLY, P3DC_LIST_LINKED};
static p3dc_LIST model_pool	= { NULL, NULL,	P3DC_MODEL, P3DC_LIST_LINKED};
static p3dc_LIST list_pool	= { NULL, NULL,	P3DC_LIST, P3DC_LIST_LINKED};

/*
 * A type-info structure contains the P3DC type, a pointer to its pool
 * and the size of the structure needed.
 */
static struct tinfo {
	p3dc_TYPE	type;
	int			size;
	p3dc_LIST	*pool;
	struct {
		int		allocs;	/* for tracking usage */
		int		frees;	/* in memstats */
	} stats;
} info[] = {
	{ P3DC_NOTYPE,               0, &notype_pool, {0, 0} },	/* 00 */
	{ P3DC_FLOAT, sizeof(p3dc_FLOAT), &float_pool, {0, 0} },	/* 01 */
	{ P3DC_PNT2,  sizeof(p3dc_PNT2),  &pnt2_pool, {0, 0}  },	/* 02 */
	{ P3DC_PNT3,  sizeof(p3dc_PNT3),  &pnt3_pool, {0, 0}  },	/* 03 */
	{ P3DC_PNT4,  sizeof(p3dc_PNT4),  &pnt4_pool, {0, 0}  },	/* 04 */
	{ P3DC_XFRM,  sizeof(p3dc_XFRM),  &xfrm_pool, {0, 0}  },	/* 05 */
	{ P3DC_TDATA,  sizeof(p3dc_TDATA),  &tdta_pool, {0, 0}  },	/* 06 */
	{ P3DC_TXTR,  sizeof(p3dc_TXTR),  &txtr_pool, {0, 0}  },	/* 07 */
	{ P3DC_NODE,  sizeof(p3dc_NODE),  &nlst_pool, {0, 0}  },	/* 08 */
	{ P3DC_CLR,   sizeof(p3dc_CLR),   &clr_pool, {0, 0}   },	/* 09 */
	{ P3DC_VRTX,  sizeof(p3dc_VRTX),  &vrtx_pool, {0, 0}  },	/* 10 */
	{ P3DC_FACE,  sizeof(p3dc_FACE),  &face_pool, {0, 0}  },	/* 11 */
	{ P3DC_CMRA,  sizeof(p3dc_CMRA),  &cmra_pool, {0, 0}  },	/* 12 */
	{ P3DC_TYPE,  sizeof(p3dc_TYPE),  &type_pool, {0, 0}  },	/* 13 */
	{ P3DC_LIGHT,  sizeof(p3dc_LIGHT),  &lite_pool, {0, 0}  },	/* 14 */
	{ P3DC_POINT, sizeof(p3dc_POINT), &point_pool, {0, 0} },	/* 15 */
	{ P3DC_LINE,  sizeof(p3dc_LINE),  &line_pool, {0, 0}  },	/* 16 */
	{ P3DC_POLY,  sizeof(p3dc_POLY),  &poly_pool, {0, 0}  },	/* 17 */
	{ P3DC_MODEL, sizeof(p3dc_MODEL), &model_pool, {0, 0} },	/* 18 */
	{ P3DC_LIST, sizeof(p3dc_LIST), &list_pool, {0, 0} },		/* 19 */
	{ 0, 0, NULL, {0, 0} },		/* End of list Marker */
};

/*
 * type_info()
 *
 * This function quickly returns a type-info structure for the
 * given type. It uses a switch statement to be fast (vs a linear
 * search of the info[] array.
 */
static struct tinfo *
type_info(p3dc_TYPE t) {
	switch (t) {
	case P3DC_NOTYPE:
		return &info[0];
	case P3DC_FLOAT:
		return &info[1];
	case P3DC_PNT2:
		return &info[2];
	case P3DC_PNT3:
		return &info[3];
	case P3DC_PNT4:
		return &info[4];
	case P3DC_XFRM:
		return &info[5];
	case P3DC_TDATA:
		return &info[6];
	case P3DC_TXTR:
		return &info[7];
	case P3DC_NODE:
		return &info[8];
	case P3DC_CLR:
		return &info[9];
	case P3DC_VRTX:
		return &info[10];
	case P3DC_FACE:
		return &info[11];
	case P3DC_CMRA:
		return &info[12];
	case P3DC_TYPE:
		return &info[13];
	case P3DC_LIGHT:
		return &info[14];
	case P3DC_POINT:
		return &info[15];
	case P3DC_LINE:
		return &info[16];
	case P3DC_POLY:
		return &info[17];
	case P3DC_MODEL:
		return &info[18];
	case P3DC_LIST:
		return &info[19];
	default:
		return NULL;
	}
}

static char *
type_name(p3dc_TYPE t) {
	switch (t) {
	case P3DC_NOTYPE:
		return "NOTYPE";
	case P3DC_FLOAT:
		return "FLOAT";
	case P3DC_PNT2:
		return "PNT2";
	case P3DC_PNT3:
		return "PNT3";
	case P3DC_PNT4:
		return "PNT4";
	case P3DC_XFRM:
		return "XFRM";
	case P3DC_TDATA:
		return "TDATA";
	case P3DC_TXTR:
		return "TXTR";
	case P3DC_NODE:
		return "NODE";
	case P3DC_CLR:
		return "CLR";
	case P3DC_VRTX:
		return "VRTX";
	case P3DC_FACE:
		return "FACE";
	case P3DC_CMRA:
		return "CMRA";
	case P3DC_TYPE:
		return "TYPE";
	case P3DC_LIGHT:
		return "LIGHT";
	case P3DC_POINT:
		return "POINT";
	case P3DC_LINE:
		return "LINE";
	case P3DC_POLY:
		return "POLY";
	case P3DC_MODEL:
		return "MODEL";
	case P3DC_LIST:
		return "LIST";
	default:
		return "<!! UNKNOWN !!>";
	}
}

static int __new_type_called;
static int __new_forced_malloc;
static int __free_type_called;
static int __free_no_type;

void
p3dc_reset_memstats() {
	int i;

	__new_type_called = 0;
	__new_forced_malloc  = 0;
	__free_type_called = 0;
	__free_no_type = 0;
	for (i = 0; info[i].pool != NULL; i++) {
		info[i].stats.allocs = info[i].stats.frees = 0;
	}
}

void
p3dc_memstats(int reset) {
	int i;

	p3dc_log("Memory statistics:\n");
	p3dc_log("    New type called %d times, (%d calls forced malloc).\n",
			__new_type_called, __new_forced_malloc);
	p3dc_log("    Free type called %d times (%d calls failed to free).\n",
			__free_type_called, __free_no_type);
	i = 0;
	while(info[i].pool != NULL) {
		if ((info[i].stats.allocs != 0) || (info[i].stats.frees != 0))
			p3dc_log("	-- %s [%d/%d/%d] (allocs/frees/difference)\n", type_name(info[i].type),
						info[i].stats.allocs, info[i].stats.frees,
						info[i].stats.allocs - info[i].stats.frees);
		i++;
	}
	if (reset)
		p3dc_reset_memstats();
}

/*
 * Allocate a new P3DC Type structure.
 *
 * This code first checks to see if the pool of free structures
 * of this type has any available structures, if so it returns
 * one, else it allocates one.
 */
void *
p3dc_new_type(p3dc_TYPE t) {
	p3dc_NODE	*r;
	struct tinfo *ti = type_info(t);

	__new_type_called++;
	if (ti == NULL)
		return NULL;

	assert(ti->pool->n_type == t);

	ti->stats.allocs++;
	if (ti->pool->head != NULL) {
		r = p3dc_remove_node(ti->pool, P3DC_NODE_FIRST);
	} else {
		__new_forced_malloc++;
		r = (p3dc_NODE *)malloc(NODE_SIZE + ti->size);
		r->data.p = ((char *) r) + NODE_SIZE;
		r->data.t = t;
		r->owner = NULL;
		r->name = NULL;
		r->nxt = r->prv = NULL;
		r->parent = NULL;
	}
	return (r->data.p);
}

/*
 * Free a type, this 'backs up' the pointer to get back to the p3dc_NODE
 * structure, verifies that we are in fact the correct NODE for this
 * type and then adds the type back into the Pool of free structures.
 */
void
p3dc_free_type(void *payload) {
	p3dc_NODE *n;
	struct tinfo *ti;

	__free_type_called++;
	if (payload == NULL)
		return;

	n = (void *)(((char *)payload) - NODE_SIZE);
	if (n->data.p != payload)
		return;

	/* Can't free "unknown" types */
	if (n->data.t == P3DC_UNKNOWN) 
		return; 

	ti = type_info(n->data.t);
	if (ti == NULL) {
		__free_no_type++;
		return; /* wasn't a recognized type (could be an assert)*/
	}
	ti->stats.frees++;
	/* n->owner != NULL means it is already on the free list! */
	assert(n->owner == NULL); 
	p3dc_add_node(ti->pool, n, P3DC_LIST_HEAD);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * */
