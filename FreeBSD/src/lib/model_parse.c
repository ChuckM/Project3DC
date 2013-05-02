/*
 * model_parse.c -- P3DC Model Description Language Parser
 * $Id: model_parse.c,v 1.5 1999-10-31 11:24:00-08 cmcmanis Exp cmcmanis $
 * 
 * This is the model description language (MDL) parser. It
 * parses model descriptions into instances of a p3dc_MODEL
 * structure.
 *
 * NOTE: Use of FILE * is currently NYI, using integer
 *		 fd's and the open and read calls.
 *
 * The only global functions in this file are:
 *		p3dc_new_model(char *name, float scale);
 *			Fetch a new model from the current list
 *			of available models, try to read it if it
 *			isn't in the list.
 *
 *		p3dc_free_model(p3dc_MODEL *model);
 *			Return model resources to the free pool.
 *
 *		p3dc_dump_model(FILE *dump);
 *			"Unparse" or decompile a model structure back
 *			into MDL and output it to 'dumpfile' or the
 *			session.log file is dumpfile is NULL.
 *
 *		p3dc_parse_model(FILE *input);
 *			parse a model from the currently open file.
 *
 *
 * Copyright statement:
 *
 * All of the documentation and software included in Project: 
 * 3D Craft is copyrighted by Chuck McManis.
 *
 *   Copyright (c) 1997-1999 Chuck McManis, all rights
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
 *	$Log: model_parse.c,v $
 *	Revision 1.5  1999-10-31 11:24:00-08  cmcmanis
 *	Camera rewrite is nearly complete. This seems like a logical
 *	milestone
 *
 *	Revision 1.4  1999/09/12 06:30:52  cmcmanis
 *	Rewrote clipping api (again?)
 *
 *	Revision 1.2  1999/09/02 03:22:43  cmcmanis
 *	The great LIST rewrite, lists are much cleaner now
 *
 *	Revision 1.1  1999/08/18 01:33:51  cmcmanis
 *	Initial revision
 *
 *	Revision 1.1  1999/08/18 01:06:17  cmcmanis
 *	Initial revision
 *
 *	Revision 1.7  1999/08/16 09:51:26  cmcmanis
 *	Woo-hoo. Textures work again.
 *
 *	Revision 1.6  1999/08/14 23:18:38  cmcmanis
 *	This version now includes a new model structure that allows
 *	for dynamic changing of colors and textures in the model
 *	after it has been loaded.
 *
 *	Revision 1.5  1999/07/25 09:11:54  cmcmanis
 *	The great renaming from g3d/G3D -> p3dc/P3DC
 *
 *	Revision 1.4  1998/11/16 03:41:00  cmcmanis
 *	Updated clipping
 *
 *	Revision 1.4  1998/11/08 05:21:05  cmcmanis
 *	Outdated (renamed to 'H:\P3DC-RCS\src\lib\model_parse.c,v').
 *
 *	Revision 1.3  1998/11/05 20:23:43  cmcmanis
 *	Checkin prior to moving into a better source tree structure
 *
 *	Revision 1.2  1998/10/18 09:29:46  cmcmanis
 *	Models now position and move correctly. I've re-ordered my matrices
 *	to be SPHIGs compatible so that I can use SPHIGS to test my results.
 *	Created p3dc_camera_xfrm and p3dc_orientation_xfrm as compliments of
 *	each other. Cool stuff there.
 *
 *	Revision 1.1  1998/10/13 21:04:17  cmcmanis
 *	Initial revision
 * 
 *	Revision 0.9  1998/10/08 08:33:55  cmcmanis
 *	Derived from the initial model.c file.
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifdef __MSC__
#include <fcntl.h>
#include <io.h>
#define strcasecmp _stricmp
#else
#include <sys/fcntl.h>
#include <unistd.h>
#define _O_RDONLY O_RDONLY
#define _open open
#define _read read
#define _close close
#endif
#include <assert.h>
#include <math.h>
#include "p3dc.h"
/*
 * this enum defines the set of tokens that makes up our
 * simplistic model structure.
 */
typedef enum {
	MODEL_NONE = 0,
	MODEL_UNKNOWN,
	MODEL_COLOR,
	MODEL_COMMA,
	MODEL_END,
	MODEL_EOF,
	MODEL_EOL,
	MODEL_EQUAL,
	MODEL_FACE,
	MODEL_FLOAT,
	MODEL_LBRACKET,
	MODEL_LBRACE,
	MODEL_LIGHTMAP,
	MODEL_RBRACE,
	MODEL_LINE,
	MODEL_MODEL,
	MODEL_NAME,
	MODEL_NORMAL,
	MODEL_ORIGIN,
	MODEL_POINT,
	MODEL_QUOTE,
	MODEL_RBRACKET,
	MODEL_LPAREN,
	MODEL_RPAREN,
	MODEL_LANGLE,
	MODEL_RANGLE,
	MODEL_SEMI,
	MODEL_WRAP,
	MODEL_ILLUM,
	MODEL_DETAIL,
	MODEL_DECAL,
	MODEL_STRING,
	MODEL_TEXTURE,
	MODEL_VERTEX,
	MODEL_ERROR,
} model_token;

static char *
tok_name(model_token t) {
	switch (t) {
		case MODEL_NONE:
			return "MODEL_NONE";
		case MODEL_UNKNOWN:
			return "MODEL_UNKNOWN";
		case MODEL_COLOR:
			return "MODEL_COLOR";
		case MODEL_COMMA:
			return "MODEL_COMMA";
		case MODEL_END:
			return "MODEL_END";
		case MODEL_EOF:
			return "MODEL_EOF";
		case MODEL_EOL:
			return "MODEL_EOL";
		case MODEL_EQUAL:
			return "MODEL_EQUAL";
		case MODEL_FACE:
			return "MODEL_FACE";
		case MODEL_FLOAT:
			return "MODEL_FLOAT";
		case MODEL_LBRACKET:
			return "MODEL_LBRACKET";
		case MODEL_LBRACE:
			return "MODEL_LBRACE";
		case MODEL_RBRACE:
			return "MODEL_RBRACE";
		case MODEL_LINE:
			return "MODEL_LINE";
		case MODEL_MODEL:
			return "MODEL_MODEL";
		case MODEL_NORMAL:
			return "MODEL_NORMAL";
		case MODEL_ORIGIN:
			return "MODEL_ORIGIN";
		case MODEL_POINT:
			return "MODEL_POINT";
		case MODEL_QUOTE:
			return "MODEL_QUOTE";
		case MODEL_RBRACKET:
			return "MODEL_RBRACKET";
		case MODEL_LPAREN:
			return "MODEL_LPAREN";
		case MODEL_RPAREN:
			return "MODEL_RPAREN";
		case MODEL_LANGLE:
			return "MODEL_LANGLE";
		case MODEL_RANGLE:
			return "MODEL_RANGLE";
		case MODEL_SEMI:
			return "MODEL_SEMI";
		case MODEL_STRING:
			return "MODEL_STRING";
		case MODEL_TEXTURE:
			return "MODEL_TEXTURE";
		case MODEL_VERTEX:
			return "MODEL_VERTEX";
		case MODEL_DECAL:
			return "MODEL_DECAL";
		case MODEL_ERROR:
			return "MODEL_ERROR";
		case MODEL_LIGHTMAP:
			return "MODEL_LIGHTMAP";
		case MODEL_NAME:
			return "MODEL_NAME";
		case MODEL_WRAP:
			return "MODEL_WRAP";
		case MODEL_ILLUM:
			return "MODEL_ILLUM";
		case MODEL_DETAIL:
			return "MODEL_DETAIL";
		default:
			return "UNKNOWN TOKEN";
	}
	return "Unknown Token";
}

static int line_num;

#define LEX_BUF_SIZE 1024

/*
 * This is your standard (but portable) buffered file reading
 * routine. The file is read in 1K chunks into the file buffer
 * and then returned one character at a time. When the file
 * is empty the routine begins to return 0. (this precludes it
 * from being used to read binary files as there is no eof 
 * indication.
 */
static char 
__char_from_file(int file) {
	static char file_buf[LEX_BUF_SIZE];
	static int file_ndx;
	static int max_file_ndx;

	int len;
	char c;

	if (file_ndx >= max_file_ndx) {
		len = _read(file, file_buf, LEX_BUF_SIZE);
		if (len == 0) 
			return 0;
		if (len < LEX_BUF_SIZE)
			max_file_ndx = len;
		else
			max_file_ndx = 1024;
		file_ndx = 0;
	}
	c = file_buf[file_ndx];
	file_ndx++;
	return c;
}

/*
 * Now this code is layered on top of the file reading code and
 * maintains a "history" buffer that is LEX_BUF_SIZE-1 characters
 * long. This facilitates "backing up" the lexical analyzer to a
 * previous point in the input stream when it hits an ambiguous
 * token and has to go back and re-scan the input.
 *
 */
static char char_buf[LEX_BUF_SIZE];
static int char_ndx = 1;	/* Points to the last valid char in buffer */
static int mark_ndx = 1;	/* Points to the mark or earliest char */
static int cur_ndx = 0;		/* Points to the last character returned */

static char
lex_get_char(int file) {
	/* Point cur_ndx to the next char to return */
	cur_ndx = (cur_ndx + 1) % 1024; 

	/* If we've caught up to the mark, move the mark ahead */
	if (cur_ndx == mark_ndx) 
		mark_ndx = (mark_ndx + 1) % 1024;

	/* If we've caught up to the last valid character then
	 * fill in a new character and continue.
	 */
	if (cur_ndx == char_ndx) {
		char_buf[cur_ndx] = __char_from_file(file);
		char_ndx = (char_ndx+1) % 1024;
	}

	/* return the next valid character in the buffer */
	return char_buf[cur_ndx];
}

/*
 * Back the lexer up by one character is now done by simply
 * subtracing one from the current index. (this is limited
 * to 1023 back up operations of course.
 */
static void
lex_backup() {
	cur_ndx = cur_ndx - 1;
	if (cur_ndx < 0) cur_ndx = 1023;
}

/*
 * Mark our current spot in the character stream, calling
 * reset will return us to this spot.
 */
static void
lex_mark() {
	mark_ndx = cur_ndx;
}

/*
 * Reset the current spot to the last mark. We will now
 * "revisit" the characters from where we marked them
 * or the previous 1023 characters whichever is smaller.
 */
static void
lex_reset() {
	cur_ndx = mark_ndx;
}

/*
 * Return a character to the lexical analyzer, but don't
 * return whitespace (unless asked to). Primarily this
 * also deletes comments from the input stream but that
 * could be a problem with strings containing the comment
 * character (#).
 */
static char
lex_char(int file, int preserve_space) {
	char	c;
	int in_comment = 0;

	while (1) {
		c = lex_get_char(file);
		/* If we're not interpreting just return it */
		if (c == '\n') {
			line_num++;
			/* p3dc_log("Line number %d\n", line_num);  for debugging */
		}
		if (preserve_space)
			return c; 

		/* comment processing */
		if (c == '#') {
			in_comment = 1;
		} else if (c == '\n') {
			in_comment = 0;
		}

		/* if it isn't a comment and we're not whitespace */
		if ((! in_comment) && (! isspace(c))) {
			return c;
		}
	}
}

/*
 * Peek at the next non-whitespace character in the input stream.
 */
static char
lex_peek(int file) {
	char r;

	r = lex_char(file, 0);
	lex_backup();
	return r;
}

/*
 * Parse a floating point value out of the input stream. This
 * isn't particularly robust, it copies any and all -,.,0-9
 * characters into a buffer (up to a maximum) and then calls
 * atof() on it.
 */
static float
lex_float(int file, char c) {
	char buf[32];
	float res;
	int ndx = 0;

    /*
	 * while characters are valid, stuff them in the buffer
	 */
	do {
		buf[ndx++] = c;
		c = lex_char(file, 0);
	} while ((ndx < 31) && (
				(c == '.') || 
				(c == '-') || 
				((c >= '0') && (c <= '9'))));
	buf[ndx] = 0;
	res = (float) atof(buf);
	/* return the last character for the next token */
	lex_backup();
	return res;
}

/*
 * Parses a string which is defined in our grammar to be a sequence
 * of characters separated by " characters. No embedded quotes are
 * allowed.
 */
static int
lex_string(int file, char buf[], int len) {
	char c;
	int i = 0;

	/* first char was " (read in lex_token) ignore it and start reading */
	while (1) {
		c = lex_char(file, 1);
		/* end of the string (or end of line) */
		if ((c == '"') || (c == '\n')) {
			buf[i] = 0;
			return i;
		}
	
		/* out of buffer space! */
		if (i == (len  - 1)) {
			buf[i] = 0;
			return i;
		}
		buf[i++] = c;
	}
}

/*
 * Parses a name which is defined in our grammar to be a sequence
 * of characters in the range 0-9a-Z_$ and the initial character
 * *MUST* be a letter. Returns the length of name, 0 if no name.
 *
 * NB: When called, the first lex scan skips whitespace to get to
 * the first non-whitespace character, then if it is a letter, it
 * continues, now preserving the space for finding the end of the name,
 * if however the first non-whitespace character isn't a letter, it is
 * pushed back into the stream and this returns 0.
 */
static int
lex_name(int file, char buf[], int len) {
	char c;
	int i = 0;
	int keep_space = 0;

	buf[0] = 0;
	while (1) {
		c = lex_char(file, keep_space);

		/* Check to see that it starts with a letter */
		if ((i == 0) && (! isalpha(c))) {
			lex_backup(); /* Whoops! Not a name. */
			return 0;
		} else {
			/* now turn on keep_space to get the ending whitespace */
			keep_space = 1;
		}

		/* Copy in as long as its a valid name char */
		if ((! isalnum(c)) && (c != '$') && (c != '_')) {
			lex_backup();
			buf[i] = 0;
			return i;
		}

		/* out of buffer space! */
		if (i == (len  - 1)) {
			buf[i] = 0;
			return i;
		}
		buf[i++] = c;
	}
}

/*
 * Keywords list.
 * The code scans these as templates when matching the keyword, since
 * the lexer has already read the first character, it isn't included
 * and for some (Detail/Decal or Line/Lightmap) the common characters
 * are not included.
 */
static char key_origin[] = { 'r', 'i', 'g', 'i', 'n', 0 };
static char key_model[] = { 'o', 'd', 'e', 'l', 0 };
static char key_color[] = { 'o', 'l', 'o', 'r', 0 };
static char key_wrap[] = { 'r', 'a', 'p', 0 };
static char key_decal[] = { 'e', 'c', 'a', 'l', 0 };
/* Note: processed 'de' in keyword DECAL but failed looking for C */
static char key_detail[] = { 't', 'a', 'i', 'l', 0 };
static char key_vertex[] = { 'e', 'r', 't', 'e', 'x', 0};
static char key_texture[] = { 'e', 'x', 't', 'u', 'r', 'e', 0};
static char key_face[] = { 'a', 'c', 'e', 0 };
static char key_point[] = { 'o', 'i', 'n', 't', 0};
static char key_line[] = {'i', 'n', 'e', 0};
/* Note: processed 'li' in keyword but failed on third char */
static char key_lightmap[] = { 'g', 'h', 't', 'm', 'a', 'p', 0 };

/*
 * This function handles the "rest" of the token which may be
 * abbreviated. The termination cases are:
 *		the name buffer runs out of characters (it's 0)
 *		the input character isn't in the name buffer.
 */
static int
flush_token(char name_buf[], int file) {
    int local_ndx = 0;
	char local_char;

	while (1) { 
		local_char = lex_char(file, 1); 
		if (name_buf[local_ndx] == 0) {
			if (local_char != ' ')
				lex_backup();
			break;
		}

		if (tolower(local_char) != name_buf[local_ndx++]) { 
			if (local_char != ' ') 
				lex_backup(); 
			break; 
		} 
	} 
	return (local_char != ' ');	/* return 0 on OK, return 1 on error */
}

static model_token pushed_token = MODEL_NONE;
static void *pushed_data;

static void
lex_unget_token(model_token t, void *q) {
	pushed_token = t;
	pushed_data = q;
}
/*
 * This is the lexical analyzer for real, it actually does a small
 * bit of parsing (floats and strings) but for the most part it
 * just eats characters and spits tokens. (some people don't say
 * 'spit' :-)
 *
 * NB: Names (ie variables in the model def) are handled specially
 * by the parsing code when they are expected.  
 */
static model_token
lex_token(int f, void **r) {
	static char buf[80];
	static float val;
	model_token t;
	char c;

	if (pushed_token != MODEL_NONE) {
		t = pushed_token;
		pushed_token = MODEL_NONE;
		*r = pushed_data;
		return t;
	}

	*r = NULL;
	lex_mark(); /* save this spot */
	c = lex_char(f, 0);
	switch (c) {
		case 0:
			return MODEL_EOF;
		case '[' :
			return MODEL_LBRACKET;
		case ']':
			return MODEL_RBRACKET;
		case '(':
			return MODEL_LPAREN;
		case ')':
			return MODEL_RPAREN;
		case '<':
			return MODEL_LANGLE;
		case '>':
			return MODEL_RANGLE;
		case '{':
			return MODEL_LBRACE;
		case '}':
			return MODEL_RBRACE;
		case ',':
			return MODEL_COMMA;
		case '=':
			return MODEL_EQUAL;
		case '\n':
			return MODEL_EOL;
		case ';':
			return MODEL_SEMI;
		case '-':
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case '.':
			val = lex_float(f, c);
			*r = (void *)&val;
			return MODEL_FLOAT;
		case '"':
			lex_string(f, buf, 80);
			*r = (void *)buf;
			return MODEL_STRING;
		/* keywords */
		case 'o':
		case 'O':
			if (flush_token(key_origin, f)) {
				break;
			}
			return MODEL_ORIGIN;

		case 'C':
		case 'c':
			if (flush_token(key_color, f)) {
				break;
			}
			return MODEL_COLOR;

		case 'W':
		case 'w':
			if (flush_token(key_wrap, f)) {
				break;
			}
			return MODEL_WRAP;

		case 'D':
		case 'd':
			/* Could be either Decal or DETail */
			if (!flush_token(key_decal, f)) {
				return MODEL_DECAL;
			}
			/* now if it was DETail, the next char will be 't' */
			if (flush_token(key_detail, f)) {
				break; /* nope perhaps it was a name */
			}
			return MODEL_DETAIL;
		case 'M':
		case 'm':
			if (flush_token(key_model, f)) {
				break;
			}
			return MODEL_MODEL;

		/* named things, V1, V2, ... */
		case 'V':
		case 'v':
			if (flush_token(key_vertex, f)) {
				break;
			}
			return MODEL_VERTEX;

		case 'F':
		case 'f':
			if (flush_token(key_face, f)) {
				break;
			}
			return MODEL_FACE;

		/*
		 * Process the L tokens, lines and the lightmap keyword.
		 * The interpretation is "L" by itself is always the LIGHTMAP
		 * keyword since lines with no name are illegal.
		 * If the entire name matches (for the length of name) the
	  	 * string "ightmap" then its LIGHTMAP, else its LINE
	     */
		case 'L':
		case 'l':
			if (! flush_token(key_line, f)) {
				/* succeeded, so we treat it as a line. */
				return MODEL_LINE;
			}
			/* try the lightmap token, starting at the 'g' */
			if (! flush_token(key_lightmap, f)) {
				/* succeeded its a lightmap keyword */
				return MODEL_LIGHTMAP;
			}
			break;

		case 'P':
		case 'p':
			if (flush_token(key_point, f)) {
				break;
			}
			return MODEL_POINT;
		case 't':
		case 'T':
			if (flush_token(key_texture, f)) {
				break;
			}
			return MODEL_TEXTURE;
	}
	lex_reset(); /* back up to where we started */
	c = lex_peek(f); /* see if it could be a name */
	if (isalpha(c)) {
		lex_name(f, buf, 80);
		*r = buf;
		return MODEL_NAME;
	}
	return MODEL_UNKNOWN;
}

/*
 * Boiler plate code to parse a three part value :
 *		<x>, <y>, <z> 
 * where each of x, y, and z are floating point constants. 
 */
static int
parse_pnt3(int file, p3dc_PNT3 *r) {
	model_token tt;
	void *data;

	tt = lex_token(file, &data);
	if (tt != MODEL_FLOAT) {
		return 1;
	}
	r->x = *((float *)(data));

	tt = lex_token(file, &data);
	if (tt != MODEL_COMMA) {
		return 1;
	}

	tt = lex_token(file, &data);
	if (tt != MODEL_FLOAT) {
		return 1;
	}
	r->y = *((float *)(data));

	tt = lex_token(file, &data);
	if (tt != MODEL_COMMA) {
		return 1;
	}

	tt = lex_token(file, &data);
	if (tt != MODEL_FLOAT) {
		return 1;
	}
	r->z = *((float *)(data));
	return 0;
}

/*
 * Boiler plate code to parse a two part value :
 *		<u>, <v> 
 * where each of u and v are floating point constants. 
 */
static int
parse_pnt2(int file, p3dc_PNT2 *r) {
	model_token tt;
	void *data;

	tt = lex_token(file, &data);
	if (tt != MODEL_FLOAT) {
		return 1;
	}
	r->u = *((float *)(data));

	tt = lex_token(file, &data);
	if (tt != MODEL_COMMA) {
		return 1;
	}

	tt = lex_token(file, &data);
	if (tt != MODEL_FLOAT) {
		return 1;
	}
	r->v = *((float *)(data));
	return 0;
}

/*
 * Boiler plate code to parse the color statement, should be:
 *		COLOR name = red, green, blue [, alpha] ;
 * where each of red, green, and blue are floating point 
 * constants between 0 < x < 255. alpha is optional.
 */
static p3dc_NODE *
parse_color(int file) {
	model_token tt;
	p3dc_NODE *res;
	p3dc_CLR *color;
	p3dc_PNT3 value;
	char	name[64];
	unsigned char a;
	p3dc_FLOAT t;
	void *data;

	tt = lex_token(file, &data);
	if (tt != MODEL_NAME) {
		p3dc_log("Parse Error: Missing name.\n");
		return NULL;
	}
	strcpy(name, data); /* save the name */
	tt = lex_token(file, &data);
	if (tt != MODEL_EQUAL) {
		p3dc_log("Parse Error: Missing equals.\n");
		return NULL;
	}

	if (parse_pnt3(file, &value)) {
		p3dc_log("Model color values are invalid.\n");
		return NULL;
	}

	if ((value.x < 0) || (value.x > 255)) {
		p3dc_log("Model color statement, red value out of range 0 - 255\n");
		return NULL;
	}

	if ((value.y < 0) || (value.y > 255)) {
		p3dc_log("Model color statement, green value out of range 0 - 255\n");
		return NULL;
	}

	if ((value.z < 0) || (value.z > 255)) {
		p3dc_log("Model color statement, blue value out of range 0 - 255\n");
		return NULL;
	}

	tt = lex_token(file, &data);
	if (tt == MODEL_COMMA) {
		tt = lex_token(file, &data);
		if (tt != MODEL_FLOAT) {
			p3dc_log("Missing alpha value in color specification.\n");
			return NULL;
		}
		t = *((p3dc_FLOAT *)(data));
		if ((t < 0) || (t > 255)) {
			p3dc_log("Color has invalid alpha value.\n");
			return NULL;
		}
		a = (unsigned char) t;
		tt = lex_token(file, &data);
	} else {
		a = 255; 
	}

	if (tt != MODEL_SEMI) {
		p3dc_log("Model color, missing semi-colon\n");
		return NULL;
	}
	color = p3dc_new_type(P3DC_CLR);
	if (color == NULL) {
		p3dc_log("Model parse is out of memory.\n");
		return NULL;
	}
	color->gRED = (unsigned char) value.x;
	color->gGREEN = (unsigned char) value.y;
	color->gBLUE = (unsigned char) value.z;
	color->gALPHA = a;
	res = p3dc_new_node(color, name, P3DC_NODE_FREEPAYLOAD);
	if (res == NULL) {
		p3dc_free_type(color);
		p3dc_log("Model parse is out of memory.\n");
		return NULL;
	}
	return res;
}

/*
 * Boiler plate code to parse the origin statement, should be:
 *	ORIGIN <x>, <y>, <z> ;
 * where each of x, y, and z are floating point constants.
 */
static int
parse_origin(int file, p3dc_MODEL *m) {
	model_token tt;
	void *data;

	if (parse_pnt3(file, &(m->origin))) {
		p3dc_log("Model origin, error parsing values.\n");
		return 1;
	}

	tt = lex_token(file, &data);
	if (tt != MODEL_SEMI) {
		p3dc_log("Model origin, missing semi-colon\n");
		return 1;
	}
	return 0;
}

/*
 * Parse a texture specification, this has the form:
 *
 *	Texture xxxx = DECAL | DETAIL | LIGHTMAP [, WRAP] ;
 */
static p3dc_NODE *
parse_texture(int file) {
	model_token tt;
	char	name[64];
	p3dc_TDATA *txt;
	int mode = 0;
	p3dc_NODE *n;
	void *data;

	tt = lex_token(file, &data);
	if (tt != MODEL_NAME) {
		p3dc_log("Parse Error: missing name.\n");
		return NULL;
	}
	strcpy(name, data);

	tt = lex_token(file, &data);
	if (tt != MODEL_EQUAL) {
		p3dc_log("Model texture, missing = sign.\n");
		return NULL;
	}

	tt = lex_token(file, &data);
	if (tt == MODEL_DETAIL) {
		mode = P3DC_TXTR_DETAIL;
	} else if (tt == MODEL_LIGHTMAP) {
		mode = P3DC_TXTR_LIGHTMAP;
	} else if (tt == MODEL_DECAL) {
		mode = P3DC_TXTR_BASE;
	} else {
		p3dc_log("Model texture, invalid type.\n");
	}

	tt = lex_token(file, &data);
	if (tt == MODEL_COMMA) {
		tt = lex_token(file, &data);
		if (tt != MODEL_WRAP) {
			p3dc_log("Model token, unrecognized type of texture.\n");
			return NULL;
		}
		mode |= P3DC_TXTR_WRAP;
		tt = lex_token(file, &data);
	}
	if (tt != MODEL_SEMI) {
		p3dc_log("Model texture, expected semicolon.\n");
		return NULL;
	}
	txt = p3dc_new_texture_data(mode); 
	if (txt == NULL) {
		p3dc_log("Model parser out of memory.\n");
		return NULL;
	}
	n = p3dc_new_node(txt, name, P3DC_NODE_FREEPAYLOAD);
	if (n == NULL) {
		p3dc_log("Model parser out of memory.\n");
		p3dc_free_texture_data(txt);
		return NULL;
	}
	return n;
}

/*
 * Parse a vertex statement, this has the form:
 *	VERTEX name = x, y, z ;
 * Returns a NODE, suitable for use with a red/black list
 * that has a VRTX structure as its payload.
 */
static p3dc_NODE *
parse_vertex(int file) {
	p3dc_NODE	*res;
	p3dc_VRTX	*vv;
	p3dc_PNT3	value;
	model_token	tt;
	char		name[64];
	void		*data;


	tt = lex_token(file, &data);
	if (tt != MODEL_NAME) {
		p3dc_log("Parse Error: missing name.\n");
		return NULL;
	}
	strcpy(name, data);

	tt = lex_token(file, &data);
	if (tt != MODEL_EQUAL) {
		p3dc_log("Model vertex (%s), missing equal sign in vertex declaration.\n", 
					name);
		return NULL;
	}

	if (parse_pnt3(file, &value)) {
		p3dc_log("Model vertex (%s), bad declaration.\n", name);
		return NULL;
	}

	tt = lex_token(file, &data);
	if (tt != MODEL_SEMI) {
		p3dc_log("Model vertex (%s), missing semi-colon.\n", name);
		return NULL;
	}
	vv = p3dc_new_vrtx(value.x, value.y, value.z);
	res = p3dc_new_node(vv, name, P3DC_NODE_FREEPAYLOAD);
	if (res == NULL) {
		p3dc_log("Parse error: Out of Memory!\n");
		return NULL;
	}
	return res;
}

/*
 * get_part()
 * 
 * This function retrieves a named component out of the model 
 * component database after getting its name from the input
 * stream. Very useful function.
 */
static int
get_part(char name[], p3dc_MODEL *m, p3dc_TYPE expected, void **res) {
	p3dc_NODE *n;

	n = p3dc_get_node(&(m->parts), P3DC_NODE_BYNAME, name);
	if (n == NULL) {
		p3dc_log("Parse Error: Name %s is not defined in component database.\n", name);
		return 1;
	}
	if (n->data.t != expected) {
		p3dc_log("Parse Error: Component %s had wrong type.\n", name);
		return 1;
	}
	*res = n->data.p;
	return 0;
}

/*
 * Parse the remainder of a color specification which is simply
 *	color_NAME )
 * as the preceding left paren was already seen by the parser.
 * Store a pointer to the color in result. Return 1 if error.
 */
static int
parse_color_spec(int file, p3dc_MODEL *m, p3dc_CLR **res) {
	model_token tt;
	void *data;
	char name[64];

	tt = lex_token(file, &data);
	if (tt != MODEL_NAME) {
		p3dc_log("Parse Error: expected color name.\n");
		return 1;
	}
	strcpy(name, data);
	if (get_part(name, m, P3DC_CLR, (void **)(res)))
		return 1;

	tt = lex_token(file, &data);
	if (tt != MODEL_RPAREN) {
		p3dc_log("Parse Error: Missing right paren in color specification.\n");
		return 1;
	}
	return 0;
}

/*
 * Parse a point statement, which has the form:
 *		POINT ( color_NAME ) vertex_NAME ;
 */
p3dc_NODE *
parse_point(int file, p3dc_MODEL *m) {
	p3dc_NODE *res;
	model_token tt;
	p3dc_POINT	*pt;
	p3dc_CLR *clr = NULL;
	p3dc_VRTX *vtx = NULL;
	void *name;

	tt = lex_token(file, &name);
	if (tt == MODEL_LPAREN) {
		if (parse_color_spec(file, m, &clr))
			return NULL;
		else
			tt = lex_token(file, &name);
	}

	if ((tt != MODEL_NAME) || get_part(name, m, P3DC_VRTX, (void **)&vtx)) {
		p3dc_log("Parse Error: Missing vertex name in color specification.");
		return NULL;
	}

	tt = lex_token(file, &name);
	if (tt != MODEL_SEMI) {
		p3dc_log("Parse Error: Missing semi-colon.\n");
		return NULL;
	}

	pt = p3dc_new_type(P3DC_POINT);
	if (pt == NULL) {
		p3dc_log("Model point, out of memory!\n");
		return NULL;
	}
	res = p3dc_new_node(pt, NULL, P3DC_NODE_FREEPAYLOAD);
	if (res == NULL) {
		p3dc_log("Model point, out of memory!\n");
		p3dc_free_type(pt);
		return NULL;
	}

	pt->color = clr;
	pt->p0 = vtx;
	pt->lid = 0;

	/* Yahoo! We've parsed a point statement. */
	return res;
}

/*
 * Parse a line statement, which has the form:
 *		LINE ( color_NAME ) v0_NAME v1_NAME ;
 */
p3dc_NODE *
parse_line(int file, p3dc_MODEL *m) {
	model_token tt;
	p3dc_LINE	*li;
	p3dc_CLR *clr;
	p3dc_VRTX	*v0, *v1;
	p3dc_NODE *res;
	void *name;

	tt = lex_token(file, &name);
	if (tt == MODEL_LPAREN) {
		if (parse_color_spec(file, m, &clr))
			return NULL;
		else
			tt = lex_token(file, &name);
	}

	if ((tt != MODEL_NAME) || get_part(name, m, P3DC_VRTX, (void **)&v0)) {
		p3dc_log("Parse Error: Missing vertex name in LINE statement.(%d)\n", line_num);
		return NULL;
	}

	tt = lex_token(file, &name);
	if (tt != MODEL_COMMA) {
		p3dc_log("Parse Error: Missing comma between vertices on Line.(%d)\n", line_num);
		return NULL;
	}

	tt = lex_token(file, &name);
	if ((tt != MODEL_NAME) || get_part(name, m, P3DC_VRTX, (void **)&v1)) {
		p3dc_log("Parse Error: Missing vertex name in LINE statement.(%d)\n", line_num);
		return NULL;
	}

	tt = lex_token(file, &name);
	if (tt != MODEL_SEMI) {
		p3dc_log("Parse Error: Missing semi-colon in LINE statement.\n");
		return NULL;
	}

	li = p3dc_new_type(P3DC_LINE);
	if (li == NULL) {
		p3dc_log("Model line, out of memory!\n");
		return NULL;
	}
	res = p3dc_new_node(li, NULL, P3DC_NODE_FREEPAYLOAD);
	if (res == NULL) {
		p3dc_log("Model line, out of memory!\n");
		p3dc_free_type(li);
		return NULL;
	}

	li->color = clr;
	li->p1 = v0;
	li->p2 = v1;
	li->lid = 0;

	/* Yahoo! We've parsed a line statement. */
	return res;
}

/*
 * Ok, that was easy, now let's parse FACE statements. These are a bit
 * tougher since there are more variations. Fortunately we've got some
 * helper functions built so the code is straight forward if not a bit
 * complicated.
 */

/*
 * Free a FACE that was in the process of being constructed. I attempt to
 * avoid any allocations until the last second but some of the lists make
 * it inevitable.
 */
void
free_face(p3dc_FACE *f) {
/*	XXX */
}

/*
 * Parse a face statement, this has the form:
 *
 * Face [<t1 [, t2]*>] = [ (color name), ]  vertex_name  [ normal ] <u,v> <u,v>, ... ;
 *
 * Where t1, t2, ... are the names of textures
 * to apply to this face,  color is an optional base color for the polygon
 * and Vn is a vertex specification for the face <u, v> are texture 
 * coordinates.
 */
static p3dc_NODE *
parse_face(int file, p3dc_MODEL *m) {
	model_token tt;
	void		*data;
	int has_txtr, i;

	p3dc_CLR	*clr;
	p3dc_NODE	*n;
	p3dc_FACE	*face;

	/* Allocate a new FACE structure */
	face = p3dc_new_type(P3DC_FACE);
	if (face == NULL) {
		p3dc_log("Model parse is out of memory.\n");
		return NULL;
	}
	memset((char *)face, 0, sizeof(p3dc_FACE));
	/* Initialize the list types */
	p3dc_init_list(&(face->txtr), P3DC_TXTR, P3DC_LIST_LINKED);
	p3dc_init_list(&(face->vnorm), P3DC_VRTX, P3DC_LIST_LINKED);
	p3dc_init_list(&(face->poly.pn), P3DC_VRTX, P3DC_LIST_LINKED);
	face->poly.color = NULL; /* defaults to WHITE */
	face->poly.light.x = face->poly.light.y = face->poly.light.z = face->poly.light.w = 1.0;

	has_txtr = 0;
	tt = lex_token(file, &data);
	if (tt == MODEL_LANGLE) {
		while (1) {
			p3dc_TDATA *td;
			p3dc_TXTR *tx;

			tt = lex_token(file, &data);
			if (tt != MODEL_NAME)
				break;

			if (get_part(data, m, P3DC_TDATA, (void **)&td)) {
				p3dc_log("Parse Error: Undefined texture name %s.\n", data);
				return NULL;
			}
			tx = p3dc_new_face_texture(td);
			if (tx == NULL) {
				p3dc_log("Parse Error: Out of memory.\n");
				free_face(face);
				return NULL;
			}
			n = p3dc_new_node(tx, NULL, P3DC_NODE_FREEPAYLOAD);
			if (n == NULL) {
				p3dc_log("Parse Error: Out of memory.\n");
				free_face(face);
				return NULL;
			}
			has_txtr++;
			p3dc_add_node(&(face->txtr), n, P3DC_LIST_TAIL);

			tt = lex_token(file, &data);
			if (tt != MODEL_COMMA)
				break;
		}
		if (tt != MODEL_RANGLE) {
			p3dc_log("Parse Error: Missing right angle in texture list.\n");
			free_face(face);
			return NULL;
		}

		/* next will be either a name or a left paren */
		tt = lex_token(file, &data); 
	}

	/* Check to see if the optional color specification is present */
	if (tt == MODEL_LPAREN) {
		/* If it is this will initalize the color pointer */
		if (parse_color_spec(file, m, &clr)) {
			free_face(face);
			return NULL;
		} else {
			tt = lex_token(file, &data);
		}
		face->poly.color = clr;
	}

	/*
	 * Now the only thing left will be a list of vertices.
	 */
	while (1) {
		p3dc_VRTX	*v;	/* Vertex structure */
		p3dc_VRTX	*vn; /* Normal for the vertex */

		/* Attempt to read a vertex name from the input stream */
		if (tt != MODEL_NAME)
			break;

		if (get_part(data, m, P3DC_VRTX, (void **)&v)) {
			free_face(face);
			return NULL;
		}

		/* We've got our vertex, put it on the list in the poly */
		n = p3dc_new_node(v, NULL, P3DC_NODE_FREEPAYLOAD);
		if (n == NULL) {
			p3dc_log("Parse Error: Out of memory.\n");
			free_face(face);
			return NULL;
		}
		p3dc_add_node(&(face->poly.pn), n, P3DC_LIST_TAIL);

		/* Let's create a normal for this vertex */
		vn = p3dc_new_vrtx(0, 0, 0);
		if (vn == NULL) {
			p3dc_log("Parse Error: Out of memory.\n");
			return NULL;
		}
		n = p3dc_new_node(vn, NULL, P3DC_NODE_FREEPAYLOAD);
		if (n == NULL) {
			p3dc_log("Parse Error: Out of memory.\n");
			free_face(face);
			return NULL;
		}
		p3dc_add_node(&(face->vnorm), n, P3DC_LIST_TAIL);

		tt = lex_token(file, &data);
		if (tt == MODEL_LBRACKET) {
			if (parse_pnt3(file, &(vn->v))) {
				p3dc_log("Parse Error: Normal didn't parse (%d).\n", line_num);
				free_face(face);
				return NULL;
			}
			tt = lex_token(file, &data);
			if (tt != MODEL_RBRACKET) {
				p3dc_log("Parse Error: Missing close bracket for normal.\n");
				free_face(face);
				return NULL;
			}
			tt = lex_token(file, &data);
		}

		/* If there were no textures specified, we should be at the next vertex */
		if (has_txtr == 0) {
			if (tt == MODEL_SEMI)
				break;
			if (tt != MODEL_COMMA) {
				p3dc_log("Parse Error: Missing comma between vertices.\n");
				free_face(face);
				return NULL;
			}
			tt = lex_token(file, &data); /* should be a MODEL_NAME */
			continue;
		}
		 
		/* 
		 * Now parse the requisite number of texture co-ordinates 
		 * NOTE, all textures *MUST* have co-ordinates.
		 */
		for (n = FIRST_NODE(&(face->txtr)); n != NULL; 
					n = NEXT_NODE(&(face->txtr))) {
			p3dc_NODE	*n2;
			p3dc_PNT2	*tc;
			p3dc_TXTR	*tx = (p3dc_TXTR *)(n->data.p);

			if (tt != MODEL_LANGLE) {
				p3dc_log("Model parse error, missing texture coordinates.\n");
				free_face(face);
				return NULL;
			}

			tc = p3dc_new_type(P3DC_PNT2);
			if (tc == NULL) {
				p3dc_log("Parse Error: Out of memory.\n");
				free_face(face);
				return NULL;
			}
			n2 = p3dc_new_node(tc, NULL, P3DC_NODE_FREEPAYLOAD);
			if (n2 == NULL) {
				p3dc_log("Parse Error: Out of memory.\n");
				p3dc_free_type(tc);
				free_face(face);
				return NULL;
			}

			if (parse_pnt2(file, tc)) {
				p3dc_log("Model parse error, texture coordinates.\n");
				p3dc_free_node(n2);
				free_face(face);
				return NULL;
			}
			p3dc_add_node(&(tx->vrtx), n2, P3DC_LIST_TAIL);

			tt = lex_token(file, &data);
			if (tt != MODEL_RANGLE) {
				p3dc_log("Model parse error, missing texture coordinates.\n");
				free_face(face);
				return NULL;
			}
			tt = lex_token(file, &data);
		}
		if (tt == MODEL_SEMI)
			break;	/* finished with textures so this is the end */

		if (tt != MODEL_COMMA) {
			p3dc_log("Parse Error: Face statement, missing comma. (%d)\n", line_num);
		} 
		tt = lex_token(file, &data); /*next vertex name */
	}

	if (tt != MODEL_SEMI) {
		p3dc_log("Model parse error, missing semi-colon on FACE statement(%d).\n", line_num);
		free_face(face);
		return NULL;
	}

	/* ... fix up normals etc ... 
	 * XXX no colinear points XXX 
	 * TODO, first three points
	 * determine the normal, which is
	 * ok unless they are very nearly linear.
	 */
	i = 0;
	for (n = FIRST_NODE(&(face->poly.pn)); n != NULL; n = NEXT_NODE(&(face->poly.pn))) {
		p3dc_VRTX *v = (p3dc_VRTX *)(n->data.p);
		p3dc_PNT3 n1, n2, n;
		p3dc_PNT3 v0, v1, v2;

		switch (i) {
		case 0:
			v0 = v->v;
			break;
		case 1:
			v1 = v->v;
			break;
		case 2:
			v2 = v->v;
			p3dc_diff_pnt3(&v0, &v1, &n1);
			p3dc_diff_pnt3(&v0, &v2, &n2);
			p3dc_cross_pnt3(&n2, &n1, &n);
			p3dc_normalize_pnt3(&n);
			face->poly.norm = n;
			break;
		}
		i++; /* this would be good place to verify convexness */
		if (i > 2)
			break;
	}

	if (i < 3) {
		p3dc_log("Model face, insufficent vertices for a polygon.\n");
		free_face(face);
		return NULL;
	}

	for (n = FIRST_NODE(&(face->vnorm)); n != NULL; n = NEXT_NODE(&(face->vnorm))) {
		p3dc_VRTX *tv = (p3dc_VRTX *)(n->data.p);
		p3dc_PNT3 *tp = &(tv->v);
		tv->xid = 0;
		if (EQZ(tp->x) && EQZ(tp->y) && EQZ(tp->z)) {
			*tp = face->poly.norm;
		}
	}

	n = p3dc_new_node(face, NULL, P3DC_NODE_FREEPAYLOAD);
	return n;
}

/*
 * parser for statements within a model. This function
 * parses one statement and returns as follows:
 *		-1 syntax error in the statement parsed.
 *		 0 valid statement parsed, call again.
 *		 1 END statment parsed.
 */
static int
parse_statement(int file, p3dc_MODEL *m, int saw_origin, int saw_color) {
	model_token	tt;
	p3dc_NODE *n;
	void *data;

	tt = lex_token(file, &data);
	switch (tt) {
		case MODEL_VERTEX:
			n = parse_vertex(file);
			if (n == NULL)
				return -1;
			if (p3dc_add_node(&(m->parts), n, P3DC_LIST_BYNAME, n->name)) {
				p3dc_log("Model vertex, duplicate name (%s)!\n", n->name);
				return -1;
			}
			return 0;
		case MODEL_TEXTURE:
			n = parse_texture(file);
			if (n == NULL)
				return -1;
			if (p3dc_add_node(&(m->parts), n, P3DC_LIST_BYNAME, n->name))
				return -1;
			return 0;
		case MODEL_COLOR:
			n = parse_color(file);
			if (n == NULL)
				return -1;
			p3dc_add_node(&(m->parts), n, P3DC_LIST_BYNAME, n->name);
			return 0;

		case MODEL_POINT:
			n = parse_point(file, m);
			if (n == NULL)
				return -1;
			p3dc_add_node(&(m->components), n, P3DC_LIST_TAIL);
			return 0;
		case MODEL_LINE:
			n = parse_line(file,  m);
			if (n == NULL)
				return -1;
			p3dc_add_node(&(m->components), n, P3DC_LIST_TAIL);
			return 0;
		case MODEL_FACE:
			n = parse_face(file, m);
			if (n == NULL)
				return -1;
			p3dc_add_node(&(m->components), n, P3DC_LIST_TAIL);
			return 0;

		case MODEL_ORIGIN:
			if (saw_origin) {
				p3dc_log("Duplicate origin statement in model.\n");
				return -1;
			}
			saw_origin++;
			if (parse_origin(file, m))
				return -1;
			return 0;

		default:
			lex_unget_token(tt, data);
			return 1;
	}
}

/*
 * Top level parse the model entry point for the Recursive Descent
 * parser that reads in P3DC Model files.
 *
 * If the passed in parameter "model_name" points to a string then 
 * the file is scanned for a model with that name, if the string is
 * NULL then this function simply returns the next model it finds
 * in the file.
 */
static p3dc_NODE *
parse_model(int file, char *model_name) {
	char buf[80];
	int done, saw_origin, saw_color;
	p3dc_NODE *n;
	void *data;
	model_token tok;
	int got_it = 0;
	unsigned int namelen = 0;

	p3dc_MODEL *res; 

	if (model_name)
		namelen = strlen(model_name);

	/*
	 * A bit of code to find the named model
	 * in a file. The file can contain multiple models
	 * and this code just reads through them until it
	 * gets to the end.
	 */
	tok = lex_token(file, &data);
	if (tok != MODEL_MODEL) {
		p3dc_log("First token was %s\n", tok_name(tok));
		p3dc_log("Model file doesn't contain a model statment.\n");
		return NULL;
	}

	while (! got_it) {
		tok = lex_token(file, &data);
		if (tok != MODEL_NAME) {
			p3dc_log("Model is missing a name.\n");
			return NULL;
		}
		/* Match when lengths are same and case insensitive compare is true */
		if ((namelen == 0) || 
			((strlen(data) == namelen) && (strcasecmp(data, model_name) == 0))) 
			break;
		while ((tok != MODEL_MODEL) && (tok != MODEL_EOF))
			tok = lex_token(file, &data);
		if (tok == MODEL_EOF)
			return NULL;
	}
	strcpy(buf, data);

	tok = lex_token(file, &data);
	if (tok != MODEL_LBRACE) {
		p3dc_log("Model is missing opening brace.\n");
		return NULL;
	}
	res = p3dc_new_type(P3DC_MODEL);
	if (res == NULL) {
		return NULL;
	}
	memset((char *)res, 0, sizeof(p3dc_MODEL));
	res->name = strdup(buf);
	p3dc_init_list(&(res->parts), P3DC_UNKNOWN, P3DC_LIST_SORTED);
	p3dc_init_list(&(res->components), P3DC_UNKNOWN, P3DC_LIST_LINKED);
	saw_origin = saw_color = 0;
	do {
		done = parse_statement(file, res, saw_origin, saw_color);
	} while (done == 0);
	if (done < 0) 
		return NULL;

	tok = lex_token(file, &data);
	if (tok != MODEL_RBRACE) {
		p3dc_log("Model is missing closing brace (%d).\n", line_num);
	}
	n = p3dc_new_node(res, buf, P3DC_NODE_FREEPAYLOAD);
	return n;		
}

/*
 * This utility routine is used to match up the VRTX pointers
 * with the name in the Red/Black vertex tree.
 */
static char *
find_part(void *vv, p3dc_MODEL *m) {
	p3dc_LIST *vL = &(m->parts);
	p3dc_NODE *nn;
	for (nn = FIRST_NODE(vL); nn != NULL; nn = NEXT_NODE(vL)) {
		if (nn->data.p == vv)
			return nn->name;
	}
	return "**UNKNOWN PART**";
}

/*
 * The "Standard Models" array. When P3DC loads its first model
 * it reads in stdmodels.3dc and puts the models in it into this
 * tree for use by programmers.
 */
static p3dc_LIST stdmodels = { NULL, NULL, P3DC_MODEL, P3DC_LIST_SORTED, NULL };

/*
 * The "User Models" array. When P3DC loads a model and it
 * isn't one of the "standard" models, it puts it in the 
 * user model tree for re-use.
 */
static p3dc_LIST usrmodels = { NULL, NULL, P3DC_MODEL, P3DC_LIST_SORTED, NULL };

/*
 * This function creates a clone of a properly constructed model structure.
 * It also allows for some scaling to be done at the same time.
 */
static p3dc_MODEL *
copy_model(p3dc_MODEL *src, p3dc_FLOAT sx, p3dc_FLOAT sy, p3dc_FLOAT sz) {
	p3dc_MODEL *dst;
	p3dc_NODE *n1, *n2, *n3, *n4;
	p3dc_PNT3 bb_min, bb_max;

	/*
	 * Allocate a model structure.
	 */
	dst = p3dc_new_type(P3DC_MODEL);
	if (dst == NULL)
		return NULL;

	memset((char *)dst, 0, sizeof(p3dc_MODEL));
	memset((char *)&bb_min, 0, sizeof(p3dc_PNT3));
	memset((char *)&bb_max, 0, sizeof(p3dc_PNT3));
	p3dc_init_list(&(dst->parts), P3DC_UNKNOWN, P3DC_LIST_SORTED);
	p3dc_init_list(&(dst->components), P3DC_UNKNOWN, P3DC_LIST_LINKED);
	p3dc_init_list(&(dst->cache), P3DC_UNKNOWN, P3DC_LIST_LINKED);
	dst->color = src->color;
	p3dc_identity_xfrm(&(dst->O));
	p3dc_identity_xfrm(&(dst->V));
	dst->origin = src->origin;
	dst->name = strdup(src->name);

	/*
	 * Copy the RB Parts Tree of vertices into the new model while scaling
	 * vertices in the result.
	 */
	for (n1 = FIRST_NODE(&(src->parts)); n1 != NULL; n1 = NEXT_NODE(&(src->parts))) {
		if (n1->data.t == P3DC_VRTX) {
			p3dc_VRTX *vt = n1->data.p;
			p3dc_VRTX *nV;

			vt = n1->data.p;
			/* Create a new vertex with the scaled values */
			nV = p3dc_new_vrtx(vt->v.x*sx, vt->v.y*sy, vt->v.z*sz);
			/* fill out bounding box values */
			if (nV->v.x < bb_min.x) bb_min.x = nV->v.x;
			if (nV->v.x > bb_max.x) bb_max.x = nV->v.x;
			if (nV->v.y < bb_min.y) bb_min.y = nV->v.y;
			if (nV->v.y > bb_max.y) bb_max.y = nV->v.y;
			if (nV->v.z < bb_min.z) bb_min.z = nV->v.z;
			if (nV->v.z > bb_max.z) bb_max.z = nV->v.z;
			n2 = p3dc_new_node(nV, n1->name, P3DC_NODE_FREEPAYLOAD);
		} else if (n1->data.t == P3DC_CLR) {
			p3dc_CLR *clr = n1->data.p;
			p3dc_CLR *nclr;

			/* Allocate a new color structure */
			nclr = p3dc_new_type(P3DC_CLR);
			*nclr = *clr;
			n2 = p3dc_new_node(nclr, n1->name, P3DC_NODE_FREEPAYLOAD);
		} else if (n1->data.t == P3DC_TDATA) {
			p3dc_TDATA *td = n1->data.p;
			p3dc_TDATA *nT;

			/* Allocate a new texture description */
			nT = p3dc_new_type(P3DC_TDATA);
			nT->data = NULL;
			nT->mode = td->mode;
			n2 = p3dc_new_node(nT, n1->name, P3DC_NODE_FREEPAYLOAD);
		} else {
			p3dc_log("Parts array in model is corrupted.\n");
			n2 = NULL;
			abort();
		}
		/* Insert the new node into the destination's parts list */
		p3dc_add_node(&(dst->parts), n2, P3DC_LIST_BYNAME, n2->name);
	}

	/*
	 * The next list is the most complicated, its the components list. Fortunately
	 * it is also a simple linked list.
	 */
	for (n1 = FIRST_NODE(&(src->components)); n1 != NULL; n1 = NEXT_NODE(&(src->components))) {
		p3dc_POINT *oP, *nP; /* Old Point, New Point */
		p3dc_LINE *oL, *nL;	/* Old Line, New Line */
		p3dc_FACE *oF, *nF;	/* Old Face, New Face */
		int i;
		char *name;			/* Name of a vertex or texture descriptor */

		switch (n1->data.t) {
			/*
			 * Copy a point structure. Fairly easy, duplicate
			 * the struct and then "locate" the vertex to use.
			 */
			case P3DC_POINT:
				oP = n1->data.p;
				nP = p3dc_new_type(P3DC_POINT);
				if (oP->color) {
					name = find_part(oP->color, src);
					if (get_part(name, dst, P3DC_CLR, (void **)&(nP->color)))
						/* part is missing, choke? */
						p3dc_log("Part missing, named '%s'\n", name);
				} else
					nP->color = NULL;
				nP->lid = 0;
				/* This finds a node with the same vertex pointer in the vtrx list */
				name = find_part(oP->p0, src);
				if (get_part(name, dst, P3DC_VRTX, (void **)&(nP->p0)))
					p3dc_log("Part missing, named '%s'\n", name);
				n2 = p3dc_new_node(nP, NULL, P3DC_NODE_FREEPAYLOAD);
				/* Node's ready, add it into the parts list */
				p3dc_add_node(&(dst->components), n2, P3DC_LIST_TAIL);
				break;

			/*
			 * Duplicate a line structure. Its the same as a point
			 * only there are two pointers to vertices instead of
			 * just one.
			 */
			case P3DC_LINE:
				oL = n1->data.p;
				/* Allocate a new structure, copy the color, reset the lid */
				nL = p3dc_new_type(P3DC_LINE);
				if (oL->color) {
					name = find_part(oL->color, src);
					if (get_part(name, dst, P3DC_CLR, (void **)&(nL->color)))
						p3dc_log("Missing COLOR part named '%s'\n", name);
				} else {
					nL->color = NULL;
				}
				nL->lid = 0;
				name = find_part(oL->p1, src);
				if (get_part(name, dst, P3DC_VRTX, (void **)&(nL->p1)))
					p3dc_log("Missing VERTEX part named '%s'\n", name);
				name = find_part(oL->p2, src);
				if (get_part(name, dst, P3DC_VRTX, (void **)&(nL->p2)))
					p3dc_log("Missing VERTEX part named '%s'\n", name);

				/* Create a new component node consisting of a line structure */
				n2 = p3dc_new_node(nL, NULL, P3DC_NODE_FREEPAYLOAD);
				/* And add it to the list of parts...*/
				p3dc_add_node(&(dst->components), n2, P3DC_LIST_TAIL);
				break;

			/*
			 * This one isn't so easy, here we have to duplicate several
			 * lists of nodes.
			 */
			case P3DC_FACE:
				oF = n1->data.p;
				/* Allocate a new face structure */
				nF = p3dc_new_type(P3DC_FACE);
				/* Initialize it to zero (resets types of all lists!) */
				memset((char *)nF, 0, sizeof(p3dc_FACE));

				/* Initialize the polygon data */
				if (oF->poly.color) {
					name = find_part(oF->poly.color, src);
					if (get_part(name, dst, P3DC_CLR, (void **)&(nF->poly.color)))
						p3dc_log("Missing COLOR part named '%s'\n", name);
				} else {
					nF->poly.color = NULL;
				}
				nF->poly.light = oF->poly.light;
				nF->poly.lid = 0;
				p3dc_init_list(&(nF->poly.pn), P3DC_VRTX, P3DC_LIST_LINKED);
				nF->poly.norm = oF->poly.norm;
				for (i = 0; i < 5; i++) {
					nF->ancil[i] = NULL;
				}

				/*
				 * Now walk the list of polygon vertices and make
				 * references in the new polygon for each vertex.
				 * This is the same find it in the old list, then
				 * the new list exercise again.
				 */
				for (n2 = oF->poly.pn.head; n2 != NULL; n2 = n2->nxt) {
					p3dc_VRTX *vtmp;

					/* Look it up. */
					name = find_part(n2->data.p, src);
					if (get_part(name, dst, P3DC_VRTX, (void **)&(vtmp))) {
						p3dc_log("Missing VERTEX part named %s \n", name);
						continue;
					}
					/* Create a new node with it */
					n3 = p3dc_new_node(vtmp, NULL, P3DC_NODE_FREEPAYLOAD);
					/* Append it to the new vertex list */
					p3dc_add_node(&(nF->poly.pn), n3, P3DC_LIST_TAIL);
				}

				/* Now duplicate the vertex normals */
				nF->vnorm.n_type = P3DC_VRTX;
				for (n2 = oF->vnorm.head ; n2 != NULL; n2 = n2->nxt) {
					p3dc_VRTX	*op3, *np3; /* old VRTX, new VRTX */

					op3 = n2->data.p;
					np3 = p3dc_new_type(P3DC_VRTX);
					/* Straight memory copy */
					*np3 = *op3;
					n3 = p3dc_new_node(np3, NULL, P3DC_NODE_FREEPAYLOAD);
					p3dc_add_node(&(nF->vnorm), n3, P3DC_LIST_TAIL);
				}
				
				nF->lights = NULL;

				/* 
				 * Now duplicate any textures attached to this face.
				 * This not only includes duplicating the texture refs
				 * but the list of co-ordinates in each texture as
				 * well.
				 */
				nF->txtr.n_type = P3DC_TXTR;
				for (n2 = oF->txtr.head; n2 != NULL; n2 = n2->nxt) {
					p3dc_TXTR *nT, *oT = n2->data.p;

					nT = p3dc_new_type(P3DC_TXTR);
					memset((char *)nT, 0, sizeof(P3DC_TXTR));
					name = find_part(oT->tex, src);
					if (get_part(name, dst, P3DC_TDATA, (void **)&(nT->tex))) {
						p3dc_log("Missing TEXTURE part named %s\n");
						continue;
					}

					/*
					 * Now duplicate the list of texture coordinates.
					 */
					p3dc_init_list(&(nT->vrtx), P3DC_PNT2, P3DC_LIST_LINKED);
					for (n3 = oT->vrtx.head; n3 != NULL; n3 = n3->nxt) {
						p3dc_PNT2 *np2, *op2 = n3->data.p;

						np2 = p3dc_new_type(P3DC_PNT2);

						*np2 = *op2;
						n4 = p3dc_new_node(np2, NULL, P3DC_NODE_FREEPAYLOAD);
						p3dc_add_node(&(nT->vrtx), n4, P3DC_LIST_TAIL);
					}

					/* create a new node out of this texture ref */
					n3 = p3dc_new_node(nT, NULL, P3DC_NODE_FREEPAYLOAD);

					/* Add it to the face */
					p3dc_add_node(&(nF->txtr), n3, P3DC_LIST_TAIL);
				}

				/* We're duplicated the face structure! */
				n2 = p3dc_new_node(nF, NULL, P3DC_NODE_FREEPAYLOAD);
				p3dc_add_node(&(dst->components), n2, P3DC_LIST_TAIL);
				break;

			/*
			 * If the list contains any parts other than points, lines, or
			 * faces then it has been corrupted and this code aborts.
			 */
			default:
				abort();
			}
	}
	dst->corners[0].v = bb_min;
	dst->corners[1].v = bb_min;
		dst->corners[1].v.y = bb_max.y;
	dst->corners[2].v = bb_min;
		dst->corners[2].v.x = bb_max.x;
		dst->corners[2].v.y = bb_max.y;
	dst->corners[3].v = bb_min;
		dst->corners[3].v.x = bb_max.x;
	dst->corners[4].v = bb_max;
	dst->corners[5].v = bb_max;
		dst->corners[5].v.x = bb_min.x;
	dst->corners[6].v = bb_max;
		dst->corners[6].v.x = bb_min.x;
		dst->corners[6].v.y = bb_min.y;
	dst->corners[7].v = bb_max;
		dst->corners[7].v.y = bb_min.y;
	p3dc_init_list(&(dst->box), P3DC_LINE, P3DC_LIST_LINKED);
	return dst;
}

/* Prints a point in MDL format to the log */
static void
dump_point(p3dc_POINT *pp, p3dc_MODEL *m) {
	p3dc_log("    Point ");
	if (pp->color) {
		p3dc_log("( %s ) ", find_part(pp->color, m));
	}
	p3dc_log("%s ;\n", find_part(pp->p0, m));
	return;
}

/* Prints a line in MDL format to the log */
static void
dump_line(p3dc_LINE *ll, p3dc_MODEL *m) {
	p3dc_log("    Line ");
	if (ll->color) {
		p3dc_log("( %s ) ", find_part(ll->color, m));
	}
	p3dc_log("%s, ", find_part(ll->p1, m));
	p3dc_log("%s ;\n", find_part(ll->p2, m));
	return;
}

/* Prints a face in MDL format to the log */
static void
dump_face(p3dc_FACE *ff, p3dc_MODEL *m) {
	p3dc_NODE *n;
	int first_time;

	p3dc_log("    Face ");
	if (ff->txtr.head != NULL) {
		first_time = 1;
		for (n = ff->txtr.head; n != NULL; n = n->nxt) {
			p3dc_TXTR *tx = n->data.p;
			if (first_time)
				p3dc_log("< ");
			else
				p3dc_log(", ");

			p3dc_log("%s ", find_part(tx->tex, m));
		}
		p3dc_log("> ");
	}
	p3dc_log(" ( %s )\n         ", find_part(ff->poly.color, m)); 
	first_time = 1;
	for (n = ff->poly.pn.head; n != NULL; n = n->nxt) {
		p3dc_NODE *n2;
		p3dc_PNT3 p3;
		p3dc_PNT2 *p2;

		p3dc_log("%s ", find_part(n->data.p, m));
		if (first_time)
			n2 = FIRST_NODE(&(ff->vnorm));
		else
			n2 = NEXT_NODE(&(ff->vnorm));
		p3 = ((p3dc_VRTX *)(n2->data.p))->v;
		p3dc_log("[ %6.3f, %6.3f, %6.3f ] ", p3.x, p3.y, p3.z);
		for (n2 = ff->txtr.head; n2 != NULL; n2 = n2->nxt) {
			p3dc_TXTR *tx2 = n2->data.p;
			p3dc_NODE *n3;
			if (first_time)
				n3 = FIRST_NODE(&(tx2->vrtx));
			else
				n3 = NEXT_NODE(&(tx2->vrtx));
			p2 = n3->data.p;
			p3dc_log(" < %6.3f, %6.3f >", p2->u, p2->v);
		}
		first_time = 0;
		if (n->nxt != NULL)
			p3dc_log(",\n          ");
	}
	p3dc_log(" ;\n");
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
		G L O B A L   F U N C T I O N S

  And so begins the actual exported functions in this module.

*/
p3dc_NODE *
load_model_from_file(char *filename, char *model_name) {
	int file;
	p3dc_NODE *res;

	file = _open(filename, _O_RDONLY);
	if (file == -1)
		return NULL;
	line_num = 0;
	res = parse_model(file, model_name);
	_close(file);
	return res;
}

/*
 * p3dc_new_model trys to locate the model with name 'name' and
 * return an instance of it scaled by 'scale'. All of the standard
 * models are 'unit' models, that is they are exactly 1.0 units long
 * or wide or tall. So if you want a bigger one you have to multiply
 * it by some scale. The scale is applied to all axes so the model is
 * not changed by this operation.
 */
p3dc_MODEL *
p3dc_new_model(char *name, p3dc_FLOAT sx, p3dc_FLOAT sy, p3dc_FLOAT sz) {
	char buf[80];

	p3dc_NODE *n;

	n = p3dc_get_node(&stdmodels, P3DC_NODE_BYNAME, name);
	if (n == NULL) {
		n = p3dc_get_node(&usrmodels, P3DC_NODE_BYNAME, name);
		if (n == NULL) {
			n = load_model_from_file(P3DC_STANDARD_MODELS, name);
			if (n == NULL) {
				n = load_model_from_file(P3DC_USER_MODELS, name);
				if (n == NULL) {
					strcpy(buf, name);
					strcat(buf,".3dc");
					n = load_model_from_file(buf, name);
				}
			} else {
				p3dc_add_node(&usrmodels, n, P3DC_LIST_BYNAME, n->name);
			}
		} else {
			p3dc_add_node(&stdmodels, n, P3DC_LIST_BYNAME, n->name);
		}
	}
	return (n == NULL) ? NULL : copy_model(n->data.p, sx, sy, sz);
}

/*
 * Parse a model from an open file. 
 */
p3dc_MODEL *
p3dc_parse_model(int fd, char *name) {
	p3dc_NODE *n;
	p3dc_MODEL *m;

	n = parse_model(fd, name);
	if (n != NULL) {
		m = n->data.p;
	} else
		m = NULL;
	n->data.p = NULL;
	p3dc_free_node(n);
	return m;
}

/*
 * p3dc_free_model attempts to return a model's pieces back to
 * the various places from which they come.
 * XXX need a free function for each type of component! XXX
 */
void
p3dc_free_model(p3dc_MODEL *foo) {
	p3dc_NODE *n;
	p3dc_FACE *f;

	/* Frees the vertices and their nodes. */
	p3dc_free_list(&(foo->parts));
	while (1) {
		n = p3dc_remove_node(&(foo->components), P3DC_NODE_FIRST);
		if (n == NULL)
			break;
		switch (n->data.t) {
			case P3DC_POINT:
				p3dc_free_type(n->data.p);
				break;
			case P3DC_LINE:
				p3dc_free_type(n->data.p);
				break;
			case P3DC_FACE:
				f = n->data.p;
				free_face(f);
				break;
			default:
				p3dc_log("Model is corrupt, unknown component found!\n");
				abort();
				/* not reached */
		}
		p3dc_free_node(n);
	}
	p3dc_free_type(foo);
}

/*
 * p3dc_dump_model -- convert a model back into MDL text. This routine
 * dumps a model into the log (XXX make it any file XXX) 
 */
void 
p3dc_dump_model(p3dc_MODEL *m) {
	p3dc_NODE *nn;

	p3dc_log("#\n");
	p3dc_log("# Model Dumper\n");
	p3dc_log("#\n");
	p3dc_log("model %s {\n", (m->name == NULL) ? "null" : m->name);
	p3dc_log("    origin %6.3f, %6.3f, %6.3f ;\n", m->origin.x, m->origin.y, m->origin.z);
	for (nn = FIRST_NODE(&(m->parts)); nn != NULL; 
						nn = NEXT_NODE(&(m->parts))) {
		p3dc_VRTX *v;
		p3dc_CLR *c;
		p3dc_TDATA *t;
		
		switch (nn->data.t) {
		case P3DC_VRTX :
			v = nn->data.p;
			p3dc_log("  Vertex %s = %6.3f, %6.3f, %6.3f ;\n", nn->name, v->v.x, v->v.y, v->v.z);
			break;
		case P3DC_CLR:
			c = nn->data.p;
			p3dc_log("  Color %s = %4u, %4u, %4u, %4u ;\n", nn->name, 
						c->gRED, c->gGREEN, c->gBLUE, c->gALPHA);
			break;
		case P3DC_TDATA:
			t = nn->data.p;
			p3dc_log("  Texture %s = ", nn->name);
			if (t->mode & P3DC_TXTR_BASE)
				p3dc_log("DECAL");
			else if (t->mode & P3DC_TXTR_DETAIL)
				p3dc_log("DETAIL");
			else if (t->mode & P3DC_TXTR_LIGHTMAP)
				p3dc_log("LIGHTMAP");
			if (t->mode & P3DC_TXTR_WRAP)
				p3dc_log(", WRAP");
			p3dc_log(";\n");
			break;
		default:
			p3dc_log("Model is corrupt!\n");
			abort();
			/* NOT REACHED */
		}
	}
	for (nn = m->components.head; nn != NULL; nn = nn->nxt) {
		switch (nn->data.t) {
		case P3DC_POINT:
			dump_point((p3dc_POINT *)(nn->data.p), m); 
			break;
		case P3DC_LINE:
			dump_line((p3dc_LINE *)(nn->data.p), m);
			break;
		case P3DC_FACE:
			dump_face((p3dc_FACE *)(nn->data.p), m);
			break;
		default:
			abort();
		}
	}
	p3dc_log("}\n");
}	
