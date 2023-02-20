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

#include <stdlib.h>

#include <charset.h>

struct charset *new_charset(FT_Face face) {
	struct charset *ret;
	ret = malloc(sizeof *ret);
	if (ret == NULL) {
		return NULL;
	}
	ret->face = face;
	ret->first = NULL;
	ret->last = &ret->first;
	ret->size = 0;
	return ret;
}

void free_charset(struct charset *charset) {
	struct glyph *glyph = charset->first;
	while (glyph != NULL) {
		struct code *iter = glyph->code;
		while (iter != NULL) {
			struct code *next_iter = iter->next;
			free(iter);
			iter = next_iter;
		}

		struct glyph *next_glyph;
		next_glyph = glyph->next;
		free(glyph);
		glyph = next_glyph;
	}
	free(charset);
}

int add_char(struct charset *charset, FT_ULong charcode) {
	struct glyph *new_glyph;
	struct code *first_code;
	FT_UInt gindex;

	if (charset->size == 32 && charcode != 32) {
		if (add_char(charset, 32)) {
			return 1;
		}
	}
	/* XXX: The character in position 32 MUST be ASCII ' ', as the kernel
	 * doesn't check the unicode table to figure out what the blank
	 * character is. */

	new_glyph = malloc(sizeof *new_glyph);
	if (new_glyph == NULL) {
		return 1;
	}
	first_code = malloc(sizeof *first_code);
	if (first_code == NULL) {
		free(new_glyph);
		return 1;
	}

	first_code->charcode = charcode;
	first_code->next = NULL;

	gindex = FT_Get_Char_Index(charset->face, charcode);
	new_glyph->index = gindex;
	new_glyph->code = first_code;
	new_glyph->last_code = &new_glyph->code->next;
	new_glyph->next = NULL;
	*charset->last = new_glyph;
	charset->last = &new_glyph->next;
	++charset->size;
	return 0;
}

int add_equivalent(struct glyph *glyph, FT_ULong charcode) {
	struct code *new_code;
	new_code = malloc(sizeof *new_code);
	if (new_code == NULL) {
		return -1;
	}
	new_code->charcode = charcode;
	new_code->next = NULL;
	*glyph->last_code = new_code;
	glyph->last_code = &new_code->next;
	return 0;
}

struct glyph *search_glyph(struct charset *charset, FT_ULong charcode) {
	struct glyph *g_iter = charset->first;
	while (g_iter != NULL) {
		struct code *c_iter = g_iter->code;
		while (c_iter != NULL) {
			if (c_iter->charcode == charcode) {
				return g_iter;
			}
			c_iter = c_iter->next;
		}
		g_iter = g_iter->next;
	}
	return NULL;
}
