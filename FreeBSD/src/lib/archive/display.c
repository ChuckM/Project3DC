/*
 * display.c - Display control function
 * $Id: display.c,v 1.1 1999/08/18 01:33:39 cmcmanis Exp cmcmanis $
 *
 * This function implements what are considered "generic" functions on the display
 * hardware.
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
#include <stdarg.h>
#include "p3dc.h"

#define DEFAULT_FOG 0x3f3fc8ff

p3dc_CLR __p3dc_fog_color = {DEFAULT_FOG};

/*
 * p3dc_display_ctrl() - manipulate the display.
 */
int
p3dc_display_ctrl(int func, ...) {
	va_list	vl;
	p3dc_CLR *c;
	int r = 0;

	va_start(vl, func);
	if (func == P3DC_CTRL_FOGCOLOR) {
		c = va_arg(vl, p3dc_CLR *);
		if (c == NULL)
			__p3dc_fog_color.rgba = DEFAULT_FOG;
		else
			__p3dc_fog_color.rgba = c->rgba;
	} else {
		r = __hw_control(func, vl);
	}
	va_end(vl);
	return r;
}