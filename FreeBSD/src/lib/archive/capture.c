/*
 * capture.c - This module has the screen capture code for the P3DC library.
 * $Id: capture.c,v 1.1 1999/08/18 01:33:36 cmcmanis Exp cmcmanis $
 *
 * This module implements the screen capture function. This is pretty cool
 * since you can capture a screen image easily and then display it on your
 * web site. I use the Portable Network Graphics library to generate the
 * image file.
 *
 * Change Log:
 *	$Log: capture.c,v $
 *	Revision 1.1  1999/08/18 01:33:36  cmcmanis
 *	Initial revision
 *
 *	Revision 1.1  1999/08/18 01:05:57  cmcmanis
 *	Initial revision
 *
 *	Revision 1.2  1999/08/16 09:51:21  cmcmanis
 *	Woo-hoo. Textures work again.
 *
 *	Revision 1.1  1999/08/14 23:18:56  cmcmanis
 *	Initial revision
 *
 * Copyright statement:
 *
 * All of the documentation and software included in the Project
 * 3D Craft library is copyrighted by Chuck McManis.
 *
 *   Copyright (c) 1997-1999 Chuck McManis, all rights
 *   reserved. 
 * 
 * Redistribution and use in source and binary forms, with or
 * without modification, is permitted provided that the terms
 * specified in the file license.txt are met.
 *
 * If the license file is not provided terms of the license may
 * be received by sending email to "p3dc_license@mcmanis.com"
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
#include <time.h>
#include <setjmp.h>
#include "p3dc.h"
#include "writepng.h"

static mainprog_info snap;
static unsigned char	imagedata[640*3]; /* 640 pixels, three bytes per pixel */

static int
invalid_latin1(unsigned char *p)
{
	while ((*p == 10) || ((*p > 31) && (*p < 127)) || (*p > 160)) {
		p++;
	}
    return (*p != 0);
}

/*
 * Take a snap shot of the current display. This should be called just _after_
 * you call flip frame as it will take the copy of the front display and put
 * it into the file.
 *
 * Note, I "cheat" and select a bunch of stuff to make for good pictures. That
 * includes pixel format etc. The PNG format can hold much more complex stuff
 * than this as demonstrated in the texture reading code.
 *
 * This code calls the routines from the book "PNG: The Definitive Guide"
 *
 * Important, snap is cleared so don't leave any allocated memory hanging on it.
 *
 */
int
p3dc_snapshot(char *file, char *author, p3dc_CLR *chroma) {
	char	fname[32];
	char	copyright[72];
	int		i;

	static int fnum;	/* Frame number */

	if (file == NULL) {
		fnum++;
		sprintf(fname,"snap%03d.png", fnum);
		file = fname;
	}
	memset((char *)&snap, 0, sizeof(snap));
	snap.gamma = .58824;
	snap.width = 640;
	snap.height = 480;
	snap.sample_depth = 8; /* 8 bits per pixel (actually 565) */
	snap.pnmtype = 6; /* defined to be COLOR_RGB */
	snap.interlaced = 0;
	snap.title = "P3DC Screen Snapshot";
	snap.have_text = TEXT_TITLE;
	snap.modtime = time(NULL);
	snap.have_time = 1;
	snap.image_data = imagedata;
	if (author) {
		if (invalid_latin1((unsigned char *)author)) {
			p3dc_log("Invalid character in string '%s'\n", author);
			return 1;
		}
		snap.author = author;
		if (strlen(author) > 50) {
			p3dc_log("Author string exceeds max length of 50 characters.\n");
			return 1;
		}
		sprintf(copyright, "Copyright (c) 1999 %s", author);
		snap.copyright = copyright;
		snap.have_text |= (TEXT_COPY | TEXT_AUTHOR);
	}
	snap.outfile = fopen(file, "wb");
	if (snap.outfile == NULL) {
		p3dc_log("Unable to open file %s\n", fname);
		return 1;
	}
	/*** Todo: implement transparency 
	snap.have_bg = (chroma != NULL);
	*/
	
	if (writepng_init(&snap)) {
		p3dc_log("Unable to initialize PNG interface.\n");
		fclose(snap.outfile);
		return 1;
	}

	for (i = 0; i < 480; i++) {
		if (__hw_get_row(i, imagedata)) {
			p3dc_log("Get row failed from the GAL layer.\n");
			writepng_cleanup(&snap);
			fclose(snap.outfile);
			return 1;
		}
		if (writepng_encode_row(&snap)) {
			p3dc_log("Unable to encode row %d of image data.\n", i);
			writepng_cleanup(&snap);
			fclose(snap.outfile);
			return 1;
		}
	}
	if (writepng_encode_finish(&snap)) {
		p3dc_log("Unable to finish up with writepng.\n");
		writepng_cleanup(&snap);
		fclose(snap.outfile);
		return 1;
	}
	fclose(snap.outfile);
	return 0;
}
