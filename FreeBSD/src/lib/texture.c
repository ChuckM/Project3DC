/*
 * texture.c -- texture management for P3DC
 * $Id: texture.c,v 1.2 1999-09-01 20:22:46-07 cmcmanis Exp $ 
 * 
 * This module defines the texture management functions.
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
 *	$Log: texture.c,v $
 *	Revision 1.2  1999/09/02 03:22:46  cmcmanis
 *	The great LIST rewrite, lists are much cleaner now
 *
 *	Revision 1.1  1999/08/18 01:33:57  cmcmanis
 *	Initial revision
 *
 *	Revision 1.1  1999/08/18 01:06:21  cmcmanis
 *	Initial revision
 *
 *	Revision 1.5  1999/08/16 09:51:28  cmcmanis
 *	Woo-hoo. Textures work again.
 *
 *	Revision 1.4  1999/08/14 23:18:40  cmcmanis
 *	This version now includes a new model structure that allows
 *	for dynamic changing of colors and textures in the model
 *	after it has been loaded.
 *
 *	Revision 1.3  1999/07/25 09:11:56  cmcmanis
 *	The great renaming from g3d/G3D -> p3dc/P3DC
 *
 *	Revision 1.2  1998/11/16 03:41:03  cmcmanis
 *	Updated clipping
 *
 *	Revision 1.2  1998/11/08 05:21:53  cmcmanis
 *	Outdated (renamed to 'H:\P3DC-RCS\src\lib\texture.c,v').
 *
 *	Revision 1.1  1998/10/10 07:00:50  cmcmanis
 *	Initial revision
 *
 * 
 */

#include <string.h>
#include <stdlib.h>
#include "p3dc.h"

p3dc_TDATA *
p3dc_new_texture_data(int mode) {
	p3dc_TDATA *res;

	res = p3dc_new_type(P3DC_TDATA);
	if (res == NULL)
		return NULL;
	res->mode = mode;
	res->data = NULL;
	return res;
}

/*
 * Just a wrapper for the GAL layer to do something and give
 * us a handle to it...
 */
int
p3dc_load_texture(char *name, p3dc_TDATA *td) {
	void *res;

	res = __hw_load_texture(name, td->mode, (td->mode & P3DC_TXTR_WRAP) != 0);
	td->data = res;
	return (res == NULL); /* Return non-zero on error */
}

/*
 * Tell the hardware layer (GAL) to undo whatever it did.
 */
void
p3dc_free_texture_data(p3dc_TDATA *r) {
	if (r->data) {
		__hw_unload_texture(r->data);
	}
	p3dc_free_type(r);
}

p3dc_TXTR *
p3dc_new_face_texture(p3dc_TDATA *td) {
	p3dc_TXTR *res;

	res = p3dc_new_type(P3DC_TXTR);
	if (res == NULL)
		return NULL;
	res->tex = td;
	p3dc_init_list(&(res->vrtx), P3DC_PNT2, P3DC_LIST_LINKED);
	return res;
}

void
p3dc_free_face_texture(p3dc_TXTR *r) {
	p3dc_free_type(r);
}
