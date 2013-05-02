/*
 * log.c - implement the P3DC logging function.
 * $Id: log.c,v 1.1 1999/08/18 01:33:47 cmcmanis Exp cmcmanis $
 *
 * This module implements logging. Currently it logs to a file
 * however it will eventually log to a p3dc_CONSOLE object.
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
#include <stdarg.h>
#include "p3dc.h"

static FILE *log_file;

void
p3dc_log_close() {
	if (log_file != NULL) {
		fclose(log_file);
		log_file = NULL;
	}
}

void 
p3dc_log_flush() {
	if (log_file)
		fflush(log_file);
}

void
p3dc_log_open(char *name) {
	p3dc_log_close();
	log_file = fopen(name ? name : "session.log", "w");
}

void
p3dc_log(char *fmt, ...) {
	va_list vv;

	if (log_file == NULL)
		p3dc_log_open(NULL);

	va_start(vv, fmt);
	vfprintf(log_file, fmt, vv);
	va_end(vv);
}