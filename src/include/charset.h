// ttf2psf - A simple, working utility to generate Linux console fonts
// Copyright (C) 2023  Nate Choe
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILIY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>

#ifndef HAVE_GLYPHS
#define HAVE_GLYPHS

#include <ft2build.h>
#include FT_FREETYPE_H

struct code {
	FT_ULong charcode;
	struct code *next;
}; /* All the 32 bit character codes that correspond to a glyph */

struct glyph {
	FT_UInt index;
	struct code *code;
	struct code **last_code;
	struct glyph *next;
}; /* Linked list containing all the glyphs in a font face */

struct charset {
	FT_Face face;
	struct glyph *first;
	struct glyph **last;
	/* last = &[the last node]->next */
	int size;
};

struct charset *new_charset(FT_Face face);
void free_charset(struct charset *charset);
int add_char(struct charset *charset, FT_ULong charcode);
int add_equivalent(struct glyph *glyph, FT_ULong charcode);
struct glyph *search_glyph(struct charset *charset, FT_ULong charcode);
/* These functions return either non-zero or NULL on error. */

#endif
