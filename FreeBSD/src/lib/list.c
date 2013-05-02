/*
 * list.c - Simple list manager utilities.
 * $Id: list.c,v 1.4 1999-10-31 11:23:58-08 cmcmanis Exp cmcmanis $ 
 *
 * This module implements a simple linked list facility. These
 * lists are used all over the place in the library to hold
 * things like vertex lists and primitives to be drawn. There
 * are two basic types p3dc_LIST which is a list head with first/last
 * and type values, and p3dc_NODE which has all the pointers necessary
 * to be a singly linked list, a doubly linked list, or a btree. It
 * also has a pointer to its "payload" which is the vertex or the
 * primitive or whatever.
 * 
 * Change Log:
 *	$Log: list.c,v $
 *	Revision 1.4  1999-10-31 11:23:58-08  cmcmanis
 *	Camera rewrite is nearly complete. This seems like a logical
 *	milestone
 *
 *	Revision 1.3  1999/09/02 04:33:28  cmcmanis
 *	More list fixes, red/black lists work better now on delete
 *	there were a couple of bugs in the last few nodes to be
 *	removed...
 *
 *	Revision 1.2  1999/09/02 03:22:34  cmcmanis
 *	The great LIST rewrite, lists are much cleaner now
 *
 *	Revision 1.1  1999/08/18 01:33:45  cmcmanis
 *	Initial revision
 *
 *	Revision 1.1  1999/08/18 01:06:15  cmcmanis
 *	Initial revision
 *
 *	Revision 1.9  1999/08/16 09:51:24  cmcmanis
 *	Woo-hoo. Textures work again.
 *
 *	Revision 1.8  1999/08/14 23:18:37  cmcmanis
 *	This version now includes a new model structure that allows
 *	for dynamic changing of colors and textures in the model
 *	after it has been loaded.
 *
 *	Revision 1.7  1999/07/25 09:11:53  cmcmanis
 *	The great renaming from g3d/G3D -> p3dc/P3DC
 *
 *	Revision 1.6  1998/11/16 03:40:54  cmcmanis
 *	Updated clipping
 *
 *	Revision 1.6  1998/11/08 05:19:50  cmcmanis
 *	Outdated (renamed to 'H:\P3DC-RCS\src\lib\list.c,v').
 *
 *	Revision 1.5  1998/11/05 20:23:41  cmcmanis
 *	Checkin prior to moving into a better source tree structure
 *
 *	Revision 1.4  1998/10/13 21:04:13  cmcmanis
 *	Model parsing and basic drawing is checked out. The model positioning
 *	code and relighting code still needs work.
 *
 *	Revision 1.3  1998/10/10 07:00:37  cmcmanis
 *	Model parsing is written (not fully debugged) and the texture structures
 *	are closer to their final form. By the next check in we should have a
 *	few simple models up and running with textured models to follow.
 *
 *	Revision 1.2  1998/09/23 08:48:52  cmcmanis
 *	Clipping works! Polygon and line clipping is pretty robust at this point
 *	still looking for corner cases but I believe all of the code is correct at
 *	this point.
 *
 *	Revision 1.1  1998/09/20 08:11:05  cmcmanis
 *	Initial revision
 *
 * Copyright statement:
 *
 * All of the documentation and software included in the Project:
 * 3D Craft library is copyrighted by Chuck McManis.
 *
 *   Copyright (c) 1997-1999 Chuck McManis, all rights reserved. 
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
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "p3dc.h"

#ifdef __MSC__
#define strcasecmp _stricmp
#endif

/*
 * The define PARANOID LIST turns on some asserts that I run with
 * while developing. The debug build in MSVC defines this flag.
 *
 * Forward definitions for the red/black list functions:
 */
static p3dc_NODE *rbfirst_node(p3dc_LIST *);
static p3dc_NODE *rblast_node(p3dc_LIST *);
static p3dc_NODE *rbsucc_node(p3dc_LIST *, p3dc_NODE *);
static p3dc_NODE *rbpred_node(p3dc_LIST *, p3dc_NODE *);
static int rbadd_node(p3dc_LIST *, p3dc_NODE *);
static p3dc_NODE *rbremove_node(p3dc_LIST *, p3dc_NODE *);
static p3dc_NODE *rblookup_node(p3dc_LIST *, char *);


/*
 * Initialize an existing list structure.
 * The flag 'bst' specifies if the list
 */
void
p3dc_init_list(p3dc_LIST *list, p3dc_TYPE node_type, p3dc_LIST_TYPE list_type) {
	list->l_type = list_type;
	list->n_type = node_type;
	list->head = list->tail = list->current = NULL;
}

/*
 * Allocate a list structure from the list of free lists and then initialize
 * it.
 */
p3dc_LIST *
p3dc_new_list(p3dc_TYPE node_type, p3dc_LIST_TYPE list_type) {
	p3dc_LIST *res;

	res = p3dc_new_type(P3DC_LIST);
	if (! res)
		return NULL;

	p3dc_init_list(res, node_type, list_type);
	return res;
}

/*
 * This function will free all of the nodes in a list. It makes
 * two assumptions, one that the nodes were allocated with p3dc_new_node
 * and that the payloads were also ready to be freed. If the payload
 * type is "unknown" then it WON'T be freed (p3dc_free_type works that
 * way.)
 */
void
p3dc_free_list(p3dc_LIST *list) {
	p3dc_NODE *t;

	while ((t = p3dc_remove_node(list, P3DC_NODE_FIRST)) != NULL) {
#ifdef PARANOID_LIST
		t->nxt = (p3dc_NODE *)(0xdeadbeef);
		t->prv = (p3dc_NODE *)(0xcafebabe);
#endif
		p3dc_free_node(t);
	}
}

/*
 * Add a node to the list, there are several ways that it can be added
 * so that is verified.
 */
int
p3dc_add_node(p3dc_LIST *list, p3dc_NODE *node, p3dc_NODE_POSITION where, ... ) {
	va_list	vv;
	char *name;
	p3dc_NODE *tmp;
	
#ifdef PARANOID_LIST
	assert((list->l_type == P3DC_LIST_LINKED) || (list->l_type == P3DC_LIST_SORTED));
	if (list->n_type != P3DC_UNKNOWN)
		assert(node->data.t == list->n_type);
#endif
	switch (where) {
		case P3DC_LIST_HEAD:
#ifdef PARANOID_LIST
			assert(list->l_type != P3DC_LIST_SORTED);
#endif
			node->nxt = list->head;
			node->prv = NULL;
			list->head = node;
			node->owner = list;
			return 0;

		/*
		 * Add the passed node to the end of this list.
		 */
		case P3DC_LIST_TAIL:
#ifdef PARANOID_LIST
			assert(list->l_type != P3DC_LIST_SORTED);
#endif
			if (list->tail == NULL) {
#ifdef PARANOID_LIST
				assert(list->head == NULL);
#endif
				list->head = list->tail = node;
				node->nxt = node->prv = NULL;
			} else {
#ifdef PARANOID_LIST
				assert(list->tail->nxt == NULL);
#endif
				list->tail->nxt = node;
				node->prv = list->tail;
				node->nxt = NULL;
				list->tail = node;
			}
			node->owner = list;
			return 0;

		/*
		 * Add the passed node to the list before the indicated
		 * node.
		 */
		case P3DC_LIST_BEFORE:
			va_start(vv, where);
			tmp = va_arg(vv, p3dc_NODE *);
			va_end(vv);
#ifdef PARANOID_LIST
			assert(list->l_type != P3DC_LIST_SORTED);
			assert(tmp != NULL);
			assert(tmp->owner == list);
#endif
			if (tmp->prv == NULL) {
#ifdef PARANOID_LIST
				assert(list->head == tmp);
#endif
				list->head = node;
				node->nxt = tmp;
				tmp->prv = node;
				node->prv = NULL;
			} else {
				node->prv = tmp->prv;
				node->nxt = tmp;
				tmp->prv = node;
			}
			node->owner = list;
			return 0;

		/*
		 * Add the passed in node to the list after the
		 * indicated node.
		 */
		case P3DC_LIST_AFTER:
			va_start(vv, where);
			tmp = va_arg(vv, p3dc_NODE *);
			va_end(vv);
#ifdef PARANOID_LIST
			assert(list->l_type != P3DC_LIST_SORTED);
			assert(tmp != NULL);
			assert(tmp->owner == list);
#endif
			if (tmp->nxt == NULL) {
#ifdef PARANOID_LIST
				assert(list->tail == tmp);
#endif
				list->tail = node;
				node->prv = tmp;
				tmp->nxt = node;
				node->nxt = NULL;
			} else {
				node->prv = tmp;
				node->nxt = tmp->nxt;
				tmp->nxt = node;
			}
			node->owner = list;
			return 0;

		/*
		 * Add the node to a binary sorted tree, using the
		 * indicated name.
		 */
		case P3DC_LIST_BYNAME:
			va_start(vv, where);
			name = va_arg(vv, char *);
			va_end(vv);
#ifdef PARANOID_LIST
			assert(list->l_type == P3DC_LIST_SORTED);
			assert(name != NULL);
#endif
			if (node->name != name)
				node->name = strdup(name); /* make a copy of name to tag this node */
			rbadd_node(list, node);
			return 0;
	}
	return 1;
}


p3dc_NODE *
p3dc_remove_node(p3dc_LIST *list, p3dc_NODE_LOCATION where, ...) {
	va_list	vv;
	void	*dp;
	char	*name;
	p3dc_NODE *t;

	switch (where) {
		/* Remove the first node in the list, this is like a "pop"
		 * operation. You can implement a LIFO queue by addinging
		 * and removing nodes with the key LIST_FIRST.
		 */
		case P3DC_NODE_FIRST:
			if (list->l_type == P3DC_LIST_LINKED) {
				if (list->head == NULL)
					return NULL;
				t = list->head;
				list->head = t->nxt;
				if (list->current == t)
					list->current = NULL;
				if (list->tail == t) {
#ifdef PARANOID_LIST
					assert(list->head == NULL);
#endif
					list->tail = NULL;
				}
			} else {
				if (list->head == NULL)
					return NULL;
				t = rbfirst_node(list);
#ifdef PARANOID_LIST
				assert(t != NULL);
#endif
				rbremove_node(list, t);
			}
			t->owner = NULL;
			return t;
			break;

		/* Remove the "last" node in the list, for binary tree lists
		 * this will be the lexically "largest" name (closer to 'zzzzz')
		 * for linked lists its just the last node.
		 */
		case P3DC_NODE_LAST:
			if (list->l_type == P3DC_LIST_LINKED) {
				if (list->tail == NULL)
					return NULL;
				t = list->tail;
				list->tail = t->prv;
				if (list->current == t)
					list->current = NULL;
				if (list->head == t) {
#ifdef PARANOID_LIST
					assert(list->tail == NULL);
#endif
					list->head = NULL;
				}
			} else {
				t = rblast_node(list);
				if (t == NULL)
					return NULL;
				rbremove_node(list, t);
				/* XXX kind of bogus, incurs malloc/free charge on remove */
				free(t->name);
				t->name = NULL;
			}
			t->owner = NULL;
			return t;
			break;
		
		/* Remove the passed in node. */
		case P3DC_NODE_THIS:
			va_start(vv, where);
			t = va_arg(vv, p3dc_NODE *);
			va_end(vv);
#ifdef PARANOID_LIST
			assert(t != NULL);
			assert(t->owner == list);
#endif
			if (list->l_type == P3DC_LIST_LINKED) {
				if (list->head == t) {
					list->head = t->nxt;
					if (list->tail == t)
						list->tail = NULL;
					if (list->current == t)
						list->current = NULL;
					if (list->head != NULL)
						list->head->prv = NULL;
				} else if (list->tail == t) {
					list->tail = t->prv;
					if (list->head == t)
						list->head = NULL;
					if (list->current == t)
						list->current = NULL;
					if (list->tail != NULL)
						list->tail->nxt = NULL;
				} else {
					t->nxt->prv = t->prv;
					t->prv->nxt = t->nxt;
				}
			} else {
				rbremove_node(list, t);
			}
			t->owner = NULL;
			return t;

		/* Remove the current node, this is recursively called by the remove
		 * by contents case below. It's also a way to remove the node you
		 * just fetched from a list.
		 */
		case P3DC_NODE_CURRENT:
			t = list->current;
			if (t == NULL)
				return NULL;
			if (list->l_type == P3DC_LIST_LINKED) {
				list->current = NULL;
				if (list->head == t)
					list->head = t->nxt;
				if (list->tail == t)
					list->tail = t->prv;
				if (t->nxt != NULL) 
					t->nxt->prv = t->prv;
				if (t->prv != NULL)
					t->prv->nxt = t->nxt;
			} else {
				rbremove_node(list, t);
			}
			t->owner = NULL;
			return t;

		/* Only works on Binary search trees, find the node named 'name'
		 * and remove it. Returns the node or NULL if it wasn't found.
		 */
		case P3DC_NODE_BYNAME:
			va_start(vv, where);
			name = va_arg(vv, char *);
#ifdef PARANOID_LIST
			assert(name != NULL);
#endif
			t = rblookup_node(list, name);
			if (t) {
				rbremove_node(list, t);
				t->owner = NULL;
			}
			va_end(vv);
			return t;

		/* The trickiest case, search the list for a node that has
		 * a payload data pointer that matches the passed in pointer.
		 * If it is found, it will be the "current" node, so recursivly
		 * call this function to remove the current node. Now return
		 * the node pointer. 
		 */
		case P3DC_NODE_BYDATA:
			va_start(vv, where);
			dp = va_arg(vv, void *);
			for (t = p3dc_get_node(list, P3DC_NODE_FIRST); t != NULL;
			t = p3dc_get_node(list, P3DC_NODE_NEXT)) {
				if (t->data.p == dp) {
					t = p3dc_remove_node(list, P3DC_NODE_CURRENT);
					break;
				}
			}
			va_end(vv);
			return t;
		case P3DC_NODE_NEXT:
		case P3DC_NODE_PREVIOUS:
			return NULL;
	}
#ifdef PARANOID_LIST
	assert(0);
#endif
	return NULL;
}

p3dc_NODE *
p3dc_get_node(p3dc_LIST *list, p3dc_NODE_LOCATION where, ...) {
	va_list vv;
	p3dc_NODE *t;
	char *name;
	void *dp;

	switch (where) {
		/* Get the first node from the list. */
		case P3DC_NODE_FIRST:
			if (list->l_type == P3DC_LIST_LINKED) {
				t = list->current = list->head;
			} else {
				t = rbfirst_node(list);
			}
			return t;

		/* Get the last node on the list */
		case P3DC_NODE_LAST:
			if (list->l_type == P3DC_LIST_LINKED) {
				t = list->current = list->tail;
			} else {
				t = rblast_node(list);
			}
			return t;

		/* Get the current node from the list */
		case P3DC_NODE_CURRENT:
			t = list->current;
			return t;

		case P3DC_NODE_NEXT:
			if (list->current == NULL)
				return NULL;
			if (list->l_type == P3DC_LIST_LINKED) {
				t = list->current = list->current->nxt;
			} else {
				assert(list->l_type == P3DC_LIST_SORTED);
				t = list->current = rbsucc_node(list, list->current);
			} 
			return t;

		case P3DC_NODE_PREVIOUS:
			if (list->current == NULL)
				return NULL;
			if (list->l_type == P3DC_LIST_LINKED) {
				t = list->current = list->current->prv;
			} else {
				assert(list->l_type == P3DC_LIST_SORTED);
				t = list->current = rbpred_node(list, list->current);
			}
			return t;


		/* Get a named list from a searchable list */
		case P3DC_NODE_BYNAME:
#ifdef PARANOID_LIST
			assert(list->l_type == P3DC_LIST_SORTED);
#endif
			va_start(vv, where);
			name = va_arg(vv, char *);
#ifdef PARANOID_LIST
			assert(name != NULL);
#endif
			t = list->current = rblookup_node(list, name);
			va_end(vv);
			return t;
		
		/* Trickier one, return a node whose contents match the arg */
		case P3DC_NODE_BYDATA:
			va_start(vv, where);
			dp = va_arg(vv, void *);
#ifdef PARANOID_LIST
			assert(dp != NULL);
#endif
			va_end(vv);
			for (t = p3dc_get_node(list, P3DC_NODE_FIRST); t != NULL;
			t = p3dc_get_node(list, P3DC_NODE_NEXT)) {
				if (t->data.p == dp) {
					return t;
				}
			}
			return NULL;

		case P3DC_NODE_THIS:
#ifdef PARANOID_LIST
			assert(0);	/* Should never get here */
#endif
			return NULL;
	}
#ifdef PARANOID_LIST
	assert(0);	/* Should never get here */
#endif
	return NULL;
}					

/*
 * Allocate a new node. This routine will allocate a node structure
 * and then point the d pointer at the payload. The payload's type
 * is stored in the type value. This type value is used when later
 * inserting the node into lists.
 */
p3dc_NODE *
p3dc_new_node(void *payload, char *name, int flags) {
	p3dc_NODE *n = p3dc_new_type(P3DC_NODE);

	if (n == NULL)
		return NULL;

	n->nxt = n->prv = n->parent = NULL;
	n->owner = NULL;
	n->flags = P3DC_NODE_LLNODE;	/* Default to link list node */
	if (flags & P3DC_NODE_FREEPAYLOAD)
		n->flags |= P3DC_NODE_FREEPAYLOAD;

	if (name != NULL) {
		n->name = strdup(name);
		if (n->name == NULL) {
			p3dc_free_type(n);
			return NULL;
		}
	} else {
		n->name = NULL;
	}
	n->data.t = p3dc_get_type(payload);
	n->data.p = payload;
	return n;
}

/*
 * Free a node. First free the payload, then free the node structure 
 * itself. IMPORTANT: Remove the node from the list BEFORE freeing it.
 * Freeing a node will scramble its nxt/prv pointers.
 *
 * If the node's payload is another LIST, then that list is recursively
 * freed (so you can have lists of lists).
 */
void
p3dc_free_node(p3dc_NODE *node) {
	if (node->name)
		free(node->name);
	if ((node->flags & P3DC_NODE_FREEPAYLOAD) == 0) {
		/* don't worry about the payload just return */
		p3dc_free_type(node);
		return;
	}

	if (node->data.p) {
		if (node->data.t == P3DC_LIST) {
			p3dc_free_list((p3dc_LIST *) node->data.p);
		}
		p3dc_free_type(node->data.p);
	}
	p3dc_free_type(node);
}



/*
 * rbtree.c -- red/black b-tree for fast node sorting
 * $Id: list.c,v 1.4 1999-10-31 11:23:58-08 cmcmanis Exp cmcmanis $
 * 
 * This module defines a Red-Black tree (a balanced form
 * of a simple binary tree)
 */
 

#define isBlack(n)	((n == NULL) || (((n)->flags & P3DC_NODE_BLACK) != 0))
#define isRed(n)	((n != NULL) && (((n)->flags & P3DC_NODE_RED) != 0))
#define makeBlack(n)	{ (n)->flags &= ~3; (n)->flags |= P3DC_NODE_BLACK; }
#define makeRed(n)		{ (n)->flags &= ~3; (n)->flags |= P3DC_NODE_RED; }

/*
#define right 	nxt
#define left 	prv
*/

static void
rotate_right(p3dc_LIST *list, p3dc_NODE *y) {
	p3dc_NODE *x = y->prv;

	y->prv = x->nxt;
	if (x->nxt != NULL) 
		x->nxt->parent = y;
	x->parent = y->parent;
	if (y->parent == NULL) {
		list->head = x;
	} else {
		if (y == y->parent->prv)
			y->parent->prv = x;
		else
			y->parent->nxt = x;
	}
	x->nxt = y;
	y->parent = x;
}

static void
rotate_left(p3dc_LIST *list, p3dc_NODE *x) {
	p3dc_NODE *y = x->nxt;

	x->nxt = y->prv;
	if (y->prv != NULL)
		y->prv->parent = x;
	y->parent = x->parent;
	if (x->parent == NULL)
		list->head = y;
	else {
		if (x == x->parent->prv)
			x->parent->prv = y;
		else
			x->parent->nxt = y;
	}
	y->prv = x;
	x->parent = y;
}

static int
insert(p3dc_LIST *list, p3dc_NODE *z) {
	p3dc_NODE *x = list->head;
	p3dc_NODE *y = NULL;
	int c;

	makeRed(z);
	while (x != NULL) {
		y = x;
		c = strcasecmp(z->name, x->name);
		if (c == 0)
			return 1;
		x = (c < 0) ? x->prv : x->nxt;
	}
	z->parent = y;
	if (y == NULL) {
		list->head = z;
	} else {
		c = strcasecmp(z->name, y->name);
		if (c < 0)
			y->prv = z;
		else
			y->nxt = z;
	}
	return 0;
}

static p3dc_NODE *
minimum(p3dc_LIST *list, p3dc_NODE *x) {
	if (x == NULL) x = list->head;
	while (x != NULL) {
		if (x->prv == NULL)
			break;
		x = x->prv;
	}
	return x;
}

static p3dc_NODE *
maximum(p3dc_LIST *list, p3dc_NODE *x) {
	if (x == NULL) x = list->head;
	while (x != NULL) {
		if (x->nxt == NULL)
			break;
		x = x->nxt;
	}
	return x;
}

static p3dc_NODE *
rbsucc_node(p3dc_LIST *list, p3dc_NODE *x) {
	p3dc_NODE *y;

	if (x == NULL)
		return NULL;
	if (x->nxt != NULL) 
		return minimum(list, x->nxt);
	y = x->parent;
	while ((y != NULL) && (x == y->nxt)) {
		x = y;
		y = x->parent;
	}
	return y;
}

static p3dc_NODE *
rbpred_node(p3dc_LIST *list, p3dc_NODE *x) {
	p3dc_NODE *y;

	if (x == NULL)
		return NULL;
	if (x->prv != NULL)
		return maximum(list, x->prv);
	y = x->parent;
	while ((y != NULL) && (x == y->prv)) {
		x = y;
		y = x->parent;
	}
	return y;
}

static int
rbadd_node(p3dc_LIST *list, p3dc_NODE *x) {
	p3dc_NODE *y;

	if (insert(list, x))
		return 1;

#ifdef PARANOID_LIST
	assert(x->owner == NULL);
#endif
	x->owner = list;
	makeRed(x);
	while ((x->parent != NULL) && (isRed(x->parent))) {
		/* property 3 is violated at this node */
		if (x->parent == ((x->parent)->parent)->prv) {
			y = ((x->parent)->parent)->nxt;
			if ((y == NULL) || isBlack(y)) {
				if (x == x->parent->nxt) {
					x = x->parent;
					rotate_left(list, x);
				}
				makeBlack(x->parent);
				makeRed(((x->parent)->parent)); 
				rotate_right(list, (x->parent)->parent);
			} else {
				makeBlack(x->parent);
				makeBlack(y);
				makeRed((x->parent)->parent);
				x = ((x->parent)->parent);
			}
		} else {
			y = ((x->parent)->parent)->prv;
			if ((y == NULL) || isBlack(y)) {
				if (x == (x->parent)->prv) {
					x = x->parent;
					rotate_right(list, x);
				}
				makeBlack(x->parent);
				makeRed((x->parent)->parent);
				rotate_left(list, (x->parent)->parent);
			} else {
				makeBlack(x->parent); 
				makeBlack(y);
				makeRed((x->parent)->parent);
				x = (x->parent)->parent;
			}
		}
	}
	makeBlack(list->head);
	return 0;
}

static p3dc_NODE *
rblookup_node(p3dc_LIST *list, char *name) {
	p3dc_NODE *x = list->head;
	int		ci;

	while (x != NULL) {
		ci = strcasecmp(x->name, name);
		if (ci == 0)
			break;
		if (ci < 0)
			x = x->nxt;
		else
			x = x->prv;
	}
	return x;
}

static char *
blanks(int n) {
	static char buf[256];

	memset(buf, ' ', sizeof(buf));
	buf[n] = 0;
	return buf;
}

static void
print_node(p3dc_LIST *list, p3dc_NODE *x, char *indent) {
	char buf[256];

	if (x == NULL)
		return;
	if (x->nxt != NULL) {
		strcpy(buf, indent);
		if ((x->parent != NULL) && (x == x->parent->prv)) 
			strcat(buf, "|");
		else
			strcat(buf, " ");
		strcat(buf, blanks(strlen(x->name)+2));
		print_node(list, x->nxt, buf);
	}
	p3dc_log("%s%c %s", indent, isBlack(x) ? '@' : '0', x->name);
	if ((x->prv != NULL) || (x->nxt != NULL))
		p3dc_log(" +\n");
	else
		p3dc_log("\n");
	if (x->prv == NULL) {
		if (x->parent != NULL) {
			if (x == x->parent->nxt) {
				p3dc_log("%s|\n", indent);
			} else {
				p3dc_log("%s \n", indent);
			}
		}
	} else {
		strcpy(buf, indent);
		if ((x->parent != NULL) && (x == x->parent->nxt)) 
			strcat(buf,"|");
		else
			strcat(buf," ");
		strcat(buf, blanks(strlen(x->name)+2));
		p3dc_log("%s|\n", buf);
		print_node(list, x->prv, buf);
	}
}

/*
 * Re-level a redblack tree to keep the black height
 * even.
 */
static void
fixup(p3dc_LIST *list, p3dc_NODE *x) {
	p3dc_NODE *w;

	while ((x != NULL) && (x->parent != NULL) && (isBlack(x))) {
		if (x == x->parent->prv) {
			w = x->parent->nxt;
			/* Case 1: Sibling is RED */
			if (isRed(w)) {
				makeBlack(w);
				makeRed(w->parent);
				rotate_left(list, x->parent);
				w = x->parent->nxt;
			}
			/* Case 2: W has two black children */
			if (isBlack(w->prv) && isBlack(w->nxt)) {
				makeRed(w);
				x = x->parent;
			/* Case 3: One of w's children are not black */
			} else {
				if (isBlack(w->nxt)) {
					makeBlack(w->prv);
					makeRed(w);
					rotate_right(list, w);
					w = x->parent->nxt;
				}
				/* Case 4: */
				w->flags = x->parent->flags;
				makeBlack(x->parent);
				makeBlack(w->nxt);
				rotate_left(list, x->parent);
				x = list->head;
			}
		} else {
			w = x->parent->prv;
			if (isRed(w)) {
				makeBlack(w);
				makeRed(w->parent);
				rotate_right(list, x->parent);
				w = x->parent->prv;
			}
			if (isBlack(w->prv) && isBlack(w->nxt)) {
				makeRed(w);
				x = x->parent;
			} else {
				if (isBlack(w->prv)) {
					makeBlack(w->nxt);
					makeRed(w);
					rotate_left(list, w);
					w = x->parent->prv;
				}
				w->flags = x->parent->flags;
				makeBlack(x->parent);
				makeBlack(w->prv);
				rotate_right(list, x->parent);
				x = list->head;
			}
		}
	}
	if (x != NULL)
		makeBlack(x);
}

/*
 * Basic delete function with Red/Black fixup. There are three
 * cases considered when deleting a node from the tree:
 *	Case 0: The node has no children, then we just delete it.
 *	Case 1: The node has one child, then we splice the tree such
 *		that out parent that pointed to this node is given a pointer
 *		to our child.
 *	Case 2: The node has two children (thus represents the root of
 *		a subtree. In this case the successor node (which will be the
 *		new root of the tree is located and removed, that node is then
 *		inserted again where the node to be deleted is currently.
 *
 * Whenever the node that is being deleted is colored black, the tree
 * is re-leveled to maintain a consistent black height.
 */
static p3dc_NODE sentinel;

static p3dc_NODE *
rbremove_node(p3dc_LIST *list, p3dc_NODE *z) {
	p3dc_NODE *x = NULL;
	p3dc_NODE *y = NULL;
	int wasBlack;

	if (z == NULL) 
		return NULL;

	if ((z->prv == NULL) || (z->nxt == NULL)) {
		y = z;
	} else {
		y = rbsucc_node(list, z);
		/* must have only one child! */
		assert((y->nxt == NULL) || (y->prv == NULL));
	}

	if (y->prv != NULL)
		x = y->prv;
	else
		x = y->nxt;

	if (x != NULL) 
		x->parent = y->parent; /* splice out Y */
	else {
		sentinel.parent = y->parent;
		sentinel.name = "*SENTINEL*";
		makeBlack(&sentinel);
		sentinel.nxt = sentinel.prv = NULL;
		/* x = &sentinel; */
	}

	if (y->parent == NULL) {
		list->head = x;
		if (list->head == NULL)
			return y;	/* we're done only node on the list */
	} else {
		if (y == (y->parent)->prv) {
			(y->parent)->prv = x;
		} else {
			(y->parent)->nxt = x;
		}
	}
	/* save the color of Y */
	wasBlack = isBlack(y);

	/* Y is the successor, so splice it in where Z is now */
	if (y != z) {
		y->parent = z->parent;
		y->nxt = z->nxt;
		if (y->nxt != NULL)
			(y->nxt)->parent = y;
		y->prv = z->prv;
		if (y->prv != NULL)
			(y->prv)->parent = y;
		y->flags = z->flags;
		if (y->parent != NULL) {
			if (z == (z->parent)->prv) {
				(z->parent)->prv = y;
			} else {
				(z->parent)->nxt = y;
			}
		} else {
			list->head = y;
		}
		y = z; /* Now Y points at the real Z */
	}

	y->parent = y->nxt = y->prv = NULL; /* fully spliced out */
	if (wasBlack)  {
		x = (x == NULL) ? &sentinel : x;
		fixup(list, x);
		if (x == &sentinel) {
			assert(x->nxt == NULL);
			assert(x->prv == NULL);
			if (x->parent != NULL) {
				if (x->parent->prv == x)
					x->parent->prv = NULL;
				else
					x->parent->nxt = NULL;
			} else {
				list->head = NULL; /* we're done */
			}
		}
	}
			

	return y;

}

static p3dc_NODE *
rbfirst_node(p3dc_LIST *list) {
	list->current = minimum(list, list->head);
	return list->current;
}

static p3dc_NODE *
rblast_node(p3dc_LIST *list) {
	list->current = maximum(list, list->head);
	return list->current;
}

void
p3dc_rbprint_list(p3dc_LIST *list) {
	char *i = "";
	p3dc_log("----- Printing Red/Black List -----\n");
	print_node(list, list->head, i);
	p3dc_log("-------- Done Printing ------------\n");
}
