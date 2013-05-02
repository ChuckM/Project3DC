/*
 * Texture_md.c - This is the driver for the 3dfx GLIDE library. 
 *  
 * $Id: texture_md.c,v 1.1 1999-08-17 18:33:58-07 cmcmanis Exp $
 *
 * Textures are complicated enough that I thought I would put the
 * functions controlling them into a single module.
 *
 * In this module I've tried to create a texture memory management
 * system. As texture memory is fairly precious it is important to
 * use it wisely.
 *
 * The main interfaces are load_texture and set_texture, there is
 * also 'clear_texture' for completeness but I doubt it is very
 * useful.
 *
 * Copyright (c) 1998 Chuck McManis, all rights reserved.
 * May be used for commercial or non-commercial purposes provided
 * the statement "Portions of this product are copyright (c) Chuck McManis"
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <math.h>
#include <windows.h>
#include <winbase.h>
#include <glide.h>
#include "p3dc.h"

static struct TextureCatalog {
	u_long	start;
	u_long size;
	int wrap_mode;
	int tex_type;
	GrTexInfo gti;
	Gu3dfInfo info;
	char filename[64];
	int flags;
	struct TextureCatalog *next;
} *catalog;

#define MID_MEMORY	2097152

/*
 * The texture memory structure which is your basic binary
 * tree. The root of the tree is held in the variable
 * p3dc_texture_memory.
 */
struct TextureMemory {
	u_long low;
	u_long high;
	u_long size;
	struct TextureMemory *larger;
	struct TextureMemory *smaller;
	struct TextureMemory *parent;
} *p3dc_texture_memory = NULL;

static char mem_initialized = 0;

/*
 * Basic Binary Tree functions follow (all in in _node) these
 * are:
 *		min_node - return the minimum node from this one.
 *		max_node - return the largest node from this one.
 *		next_node - return the next larger node (in-order walk)	
 *		insert_node - add a new node to the tree.
 *		remove_node - remove a node from the tree.
 */
static struct TextureMemory *
min_node(struct TextureMemory *n) {
	while (n->smaller) {
		n = n->smaller;
	}
	return n;
}

static struct TextureMemory *
max_node(struct TextureMemory *n) {
	while (n->larger) {
		n = n->larger;
	}
	return n;
}

static struct TextureMemory *
next_node(struct TextureMemory *n) {
	struct TextureMemory *tm;

	if (n->larger)
		return min_node(n->larger);
	tm = n;
	while ((n->parent != NULL) && (n->parent->larger == n))
		n = n->parent;
	return (n->parent != NULL) ? n : NULL;
}


void
insert_node(struct TextureMemory *tm) {
	struct TextureMemory *n;

	/* degenerate case, no ram available to some ram available */
	n = p3dc_texture_memory;
	if (n == NULL) {
		p3dc_texture_memory = tm;
		return;
	}

	while (1) {
		if (tm->size > n->size) {
			if (n->larger != NULL)
				n = n->larger;
			else {
				n->larger = tm;
				tm->parent = n;
				return;
			}
		} else {
			if (n->smaller != NULL)
				n = n->smaller;
			else {
				n->smaller = tm;
				tm->parent = n;
				return;
			}
		}
	}
}

static void
remove_node(struct TextureMemory *tm) {
	struct TextureMemory *n;

	/* Case 0: Node has no children */
	if ((tm->larger == NULL) && (tm->smaller == NULL)) {
		if (tm->parent == NULL) {
			p3dc_texture_memory = NULL;
			return;
		}
		if (tm->parent->larger == tm)
			tm->parent->larger = NULL;
		else
			tm->parent->smaller = NULL;
		return;
	}

	/* Case 1: Node has one child 'larger' than the current node. */
	if (tm->smaller == NULL) {
		if (tm->parent == NULL) {
			p3dc_texture_memory = tm->larger;
			return;
		}
		if (tm->parent->larger == tm) 
		    tm->parent->larger = tm->larger;
		else
			tm->parent->smaller = tm->larger;
		return;
	}

	/* Case 2: Node has one child 'smaller' than the current node */
	if (tm->larger == NULL) {
		if (tm->parent == NULL) {
			p3dc_texture_memory = tm->smaller;
			return;
		}
		if (tm->parent->larger == tm) 
		    tm->parent->larger = tm->smaller;
		else
			tm->parent->smaller = tm->smaller;
		return;
	}

	/* Case 3: Node has two children.*/
	n = min_node(tm->larger);
	remove_node(n); /* Will always be case 0 or 1 above */
	n->smaller = tm->smaller;
	n->larger = tm->larger;
	n->parent = tm->parent;
	if (n->parent == NULL)
		p3dc_texture_memory = n;
}

/*
 * Now we come to the two memory functions themselves
 *	free_memory - releases memory for re-use. This function
 *		also co-alesces chunks if it can.
 *	alloc_memory - reserves some memory in the TMU for us.
 *		returns the start address of that memory or -1 if no
 *		memory is available.
 */

/*
 * This function "adds" a chunk of memory on
 * the free memory list. It allocates a structure
 * to do that.
 */
static void
free_memory(u_long start, u_long size) {
	u_long high = start + size - 1;
    struct TextureMemory *n;

	if ((start < MID_MEMORY) && ((start + size) > MID_MEMORY)) {
		p3dc_log("Bogus attempt to free memory.\n");
		return;
	}

	if (p3dc_texture_memory) {
		for (n = min_node(p3dc_texture_memory); n != NULL; n = next_node(n)) {
			if (n->high == (MID_MEMORY - 1))
				continue;

			if ((n->low == (high + 1)) || (n->high == (start - 1))) {
				remove_node(n);
				if (n->low == (high+1)) {
					n->low = start;
					n->size += size;
				} else {
					n->high = high;
					n->size += size;
				}
				insert_node(n);
				return;
			}
		}
	}

	n = (struct TextureMemory *)calloc(1, sizeof(struct TextureMemory));
	if (n == NULL) {
		p3dc_log("Unable to allocate memory for structure!\n");
		return;
	}
	n->low = start;
	n->high = start + size - 1;
	n->size = size;
	insert_node(n);
}

static u_long
my_alloc_memory(u_long size) {
	u_long res = -1;
	struct TextureMemory *n;
	
	if (mem_initialized == 0) {
		u_long min_addr;
		u_long max_addr;

		min_addr = grTexMinAddress(GR_TMU0);
		max_addr = grTexMaxAddress(GR_TMU0);
		if (max_addr > MID_MEMORY) {
			free_memory(min_addr, MID_MEMORY);
			free_memory(MID_MEMORY, (max_addr - MID_MEMORY) + 1);
		} else {
			free_memory(min_addr, (max_addr - min_addr) + 1);
		}
		mem_initialized++;
	}

	if (p3dc_texture_memory == NULL)
		return -1;

	for (n = min_node(p3dc_texture_memory); n != NULL; n = next_node(n)) {
		if (n->size >= size) {
			remove_node(n);
			res = n->low;
			n->size = n->size - size;
			if (n->size > 0) {
				n->low += size;
				insert_node(n);
			} else {
				free(n);
			}
			return res;
		}
	}
	p3dc_log("Out of texture memory!\n");
	return -1;
}

static char data_crud[252264];
static char *text_buf = &data_crud[0];
static unsigned int buf_size = 252264;

void *
__hw_load_texture(char *name, int type, int wrap) {
	Gu3dfInfo *info;
	GrTexInfo *gti;
	struct TextureCatalog *tc;

	tc = (struct TextureCatalog *)calloc(1, sizeof(struct TextureCatalog));
	if (tc == NULL)
		return NULL;
	
	info = &(tc->info);
	gti = &(tc->gti);
	tc->wrap_mode = wrap;
	tc->tex_type = type;

	if (gu3dfGetInfo(name, info) != FXTRUE) {
		return NULL;
	}

	if (buf_size < info->mem_required) {
		if (text_buf) free(text_buf);
		text_buf = malloc(info->mem_required);
		if (text_buf != NULL)
			buf_size = info->mem_required;
		else {
			buf_size = 0;
			return NULL;
		}
	}

	info->data = text_buf;

	if (gu3dfLoad(name, info) != FXTRUE) {
		p3dc_log("Texture %s failed to load.\n", name);
		return (NULL);
	}


	gti = &(tc->gti);

	gti->smallLod = info->header.small_lod;
	gti->largeLod = info->header.large_lod;
	gti->aspectRatio = info->header.aspect_ratio;
	gti->format = info->header.format;
	gti->data = info->data;
	tc->size = grTexTextureMemRequired(GR_MIPMAPLEVELMASK_BOTH, gti);
	tc->start = my_alloc_memory(tc->size);
	if (tc->start == -1) {
		p3dc_log("Unable to load texture, out of texture memory!\n");
		return NULL;
	}
	switch (gti->format) {
		case GR_TEXFMT_P_8:
		case GR_TEXFMT_AP_88:
			grTexDownloadTable( GR_TMU0, GR_TEXTABLE_PALETTE, &(info->table));
			break;
		case GR_TEXFMT_YIQ_422:
		case GR_TEXFMT_AYIQ_8422:
			grTexDownloadTable( GR_TMU0, GR_TEXTABLE_NCC1, &(info->table.nccTable));
			break;
	}
	grTexDownloadMipMap(GR_TMU0, tc->start, GR_MIPMAPLEVELMASK_BOTH, gti);
    grTexMipMapMode( GR_TMU0, GR_MIPMAP_NEAREST, FXFALSE );
	tc->next = catalog;
	catalog = tc;
	return (void *) (tc);
}

#define P3DC_TEXTURE_NONE	0
#define P3DC_TEXTURE_DECAL	1
#define P3DC_TEXTURE_SPECULAR 2
#define P3DC_TEXTURE_LIGHTMAP 3
#define P3DC_TEXTURE_BASE 4
#define P3DC_TEXTURE_DETAIL 5

static int current_texture_mode;

/*
 * Set the rendering pipeline for the specified texture mode.
 *
 * NB: This routine is _not_ thread safe !!!
 */
void
__hw_texture_mode(int mode_bits) {
	int mode = 0;

	if (mode_bits & P3DC_TXTR_BASE) {
		mode = P3DC_TEXTURE_DECAL;
	} else if (mode_bits & P3DC_TXTR_LIGHTMAP) {
		mode = P3DC_TEXTURE_LIGHTMAP;
	}
	/* accelerator, if we're already in the correct mode just return */
	if (current_texture_mode == mode)
		return;

	if (mode < 0) mode = -mode;

	current_texture_mode = mode;
	if (mode == P3DC_TEXTURE_NONE) {
		/* Just write the color from the vertices into the bitmap */
		grColorCombine(GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE,
			GR_COMBINE_LOCAL_ITERATED, GR_COMBINE_OTHER_NONE, FXFALSE);
		/* Final blend overwrites any existing pixels */
		grAlphaBlendFunction(GR_BLEND_ONE, GR_BLEND_ZERO, GR_BLEND_ONE, GR_BLEND_ZERO);
		return;
	}

	/* Set color unit to scale texture with iterated alpha */
	grColorCombine( GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_LOCAL,
                    GR_COMBINE_LOCAL_ITERATED, GR_COMBINE_OTHER_TEXTURE, FXFALSE);

	switch (mode) {
	case P3DC_TEXTURE_DECAL:
		/* Texture unit just pulls out bits and feeds them to the color combiner */
		grTexCombine(GR_TMU0, GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE,
			GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE, FXFALSE, FXFALSE);

		/* Final blend is all source, no interaction with existing pixels. */
		grAlphaBlendFunction(GR_BLEND_ONE, GR_BLEND_ZERO, GR_BLEND_ONE, GR_BLEND_ZERO);
		break;

	case P3DC_TEXTURE_SPECULAR:
		/* Texture unit just pulls out bits and feeds them to the color combiner */
		grTexCombine(GR_TMU0, GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE,
			GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE, FXFALSE, FXFALSE);

		/* Final blend is all source + destination. */
		grAlphaBlendFunction(GR_BLEND_ONE, GR_BLEND_ONE, GR_BLEND_ZERO, GR_BLEND_ZERO);
		break;

	case P3DC_TEXTURE_LIGHTMAP:
		/* Texture unit just pulls out bits and feeds them to the color combiner */
		grTexCombine(GR_TMU0, GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE,
			GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE, FXFALSE, FXFALSE);

		/* Final blend is all source * destination. */
		grAlphaBlendFunction(GR_BLEND_ZERO, GR_BLEND_DST_COLOR, GR_BLEND_ZERO, GR_BLEND_ZERO);
		break;

	case P3DC_TEXTURE_BASE:
		/* Base of a detail texture uses the detail factor */
		grTexCombine( GR_TMU0, GR_COMBINE_FUNCTION_BLEND_LOCAL, 
			GR_COMBINE_FACTOR_DETAIL_FACTOR, GR_COMBINE_FUNCTION_LOCAL,
			GR_COMBINE_FACTOR_NONE, FXFALSE,FXFALSE );

		/* Final blend we write new texture, overwriting existing pixels */
		grAlphaBlendFunction(GR_BLEND_ONE, GR_BLEND_ZERO, GR_BLEND_ONE, GR_BLEND_ZERO );
		break;

	case P3DC_TEXTURE_DETAIL:
		/* Now we use 1-detail to render the detail texture */
		grTexCombine( GR_TMU0, GR_COMBINE_FUNCTION_BLEND_LOCAL, 
			GR_COMBINE_FACTOR_ONE_MINUS_DETAIL_FACTOR, GR_COMBINE_FUNCTION_LOCAL,
			GR_COMBINE_FACTOR_NONE, FXFALSE,FXFALSE );

		/* Final blend adds what is there, with the pixel we're fetching */
		grAlphaBlendFunction(GR_BLEND_ONE, GR_BLEND_ONE, GR_BLEND_ZERO, GR_BLEND_ZERO );
		break;
	}
	return;
}

/*
 * 
 */
void
__hw_enable_texture(void *t) {
	struct TextureCatalog *tc = (struct TextureCatalog *)t;
	GrTexInfo *gti;

	gti = &(tc->gti);
	grTexSource(GR_TMU0, tc->start, GR_MIPMAPLEVELMASK_BOTH, gti);
	grTexFilterMode( GR_TMU0, GR_TEXTUREFILTER_BILINEAR, GR_TEXTUREFILTER_BILINEAR);
	if (tc->wrap_mode)
		grTexClampMode(GR_TMU0, GR_TEXTURECLAMP_WRAP, GR_TEXTURECLAMP_WRAP);
	else 
		grTexClampMode(GR_TMU0, GR_TEXTURECLAMP_CLAMP, GR_TEXTURECLAMP_CLAMP);
	__hw_texture_mode(P3DC_TEXTURE_DECAL);
}

void
__hw_disable_texture() {
	grColorCombine(GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE,
		GR_COMBINE_LOCAL_ITERATED, GR_COMBINE_OTHER_NONE, FXFALSE);
	__hw_texture_mode(P3DC_TEXTURE_NONE);
}

void
__hw_unload_texture(void *t) {
	struct TextureCatalog *tc = (struct TextureCatalog *)t;
	struct TextureCatalog *prev = NULL, *cur;

	for (cur = catalog; cur != NULL; cur = cur->next) {
		if (tc == cur) {
			if (prev = NULL) {
				catalog = cur->next;
			} else {
				prev->next = cur->next;
			}
			free_memory(tc->start, tc->size);
			free(tc);
			return;
		}
	}
}

#if 0
xxx todo figure out what to do with this 

void
__hw_texture_stats(p3dc_TextureStats *s) {
	struct TextureCatalog *tc;
	struct TextureMemory *tm;

	memset((char *)s, 0, sizeof(p3dc_TextureStats));

	for (tc = catalog; tc != NULL; tc = tc->next) {
		s->in_use += tc->size;
	}

	for (tm = min_node(p3dc_texture_memory); tm != NULL; tm = next_node(tm)) {
		s->available += tm->size;
		if (tm->size > s->largest) {
			s->largest = tm->size;
		}
		if (tm->size < s->smallest) {
			s->smallest = tm->size;
		}
	}
	return;
}
#endif

