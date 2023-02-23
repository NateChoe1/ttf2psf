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

#include <stdio.h>
#include <stdint.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <output.h>
#include <charset.h>
#include <read_unichars.h>

struct glyph *read_face(FT_Face face, int *glyph_count);
static int write_glyph(FT_Face face, int width, int height, FILE *output);
static inline int get_bit(FT_Face face,
		int row, int col, int width, int height);

int write_psf(int width, int height, struct psf_interface *interface,
		FILE *charset_file, FILE *equivalence_file,
		FT_Face face, FILE *output) {
	struct charset *charset;
	struct glyph *iter;
	FT_Error error;

	charset = new_charset(face);
	if (charset == NULL) {
		return 1;
	}
	for (;;) {
		uint32_t *line;
		int line_len;
		line = read_unichars(charset_file, &line_len);
		if (line == NULL) {
			if (line_len == 0) {
				break;
			}
			else {
				goto error;
			}
		}
		for (int i = 0; i < line_len; ++i) {
			if (add_char(charset, line[i])) {
				goto error;
			}
		}
	}
	if (charset->size > 512) {
		fprintf(stderr, "Warning: Large number of glyphs in the "
				"charset (%d)\n", charset->size);
	}
	else {
		while (charset->size != 256 && charset->size != 512) {
			if (add_char(charset, 32)) {
				goto error;
			}
		}
	}
	for (;;) {
		uint32_t *line;
		int line_len;
next_equivalence:
		line = read_unichars(equivalence_file, &line_len);
		if (line == NULL) {
			if (line_len == 0) {
				break;
			}
			else {
				goto error;
			}
		}
		for (int i = line_len - 1; i >= 0; --i) {
			struct glyph *glyph;
			glyph = search_glyph(charset, line[i]);
			if (glyph == NULL) {
				continue;
			}
			for (int j = 0; j < line_len; ++j) {
				if (j == i) {
					continue;
				}
				if (add_equivalent(glyph, line[j])) {
					goto error;
				}
			}
			goto next_equivalence;
			/* This goto is basically a 2 level continue statement,
			 * you can't do that in C all too well. */
		}
		fputs("Warning: Equivalence file includes characters not in "
				"the charset!\n", stderr);
	}
	/* Generate charset */

	if (interface->write_header(width, height, 1, charset->size, output)) {
		goto error;
	}
	/* Write header */

	iter = charset->first;
	while (iter != NULL) {
		error = FT_Load_Glyph(face, iter->index,
				FT_LOAD_RENDER | FT_LOAD_MONOCHROME);
		if (error) {
			goto error;
		}
		if (face->glyph->format != FT_GLYPH_FORMAT_BITMAP ||
			face->glyph->bitmap.pixel_mode != FT_PIXEL_MODE_MONO) {
			goto error;
		}
		if (write_glyph(face, width, height, output)) {
			goto error;
		}
		iter = iter->next;
	}
	/* Write glyphs */

	iter = charset->first;
	while (iter != NULL) {
		struct code *code = iter->code;
		while (code != NULL) {
			if (interface->write_unichar(code->charcode, output)) {
				goto error;
			}
			code = code->next;
		}
		if (interface->write_separator(output)) {
			goto error;
		}
		iter = iter->next;
	}
	/* Write unicode table */

	free_charset(charset);
	return 0;
error:
	free_charset(charset);
	return 1;
}

static int write_glyph(FT_Face face, int width, int height, FILE *output) {
	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < (width+7) / 8; ++j) {
			unsigned char curr_char;
			curr_char = 0x00;
			for (int k = 0; k < 8; ++k) {
				curr_char <<= 1;
				curr_char |= get_bit(face,
						i, j*8 + k, width, height);
			}
			if (fputc(curr_char, output) == EOF) {
				return 1;
			}
		}
	}
	return 0;
}

static inline int get_bit(FT_Face face,
		int row, int col, int width, int height) {
	const FT_GlyphSlot glyph = face->glyph;
	const FT_Bitmap *bmp = &glyph->bitmap;
	const FT_UShort 
		//x_ppem = face->size->metrics.x_ppem,
		// For now x_ppem isn't used
		y_ppem = face->size->metrics.y_ppem;
	const int max_height = face->bbox.yMax * y_ppem / face->units_per_EM;
	const int
		left_edge = glyph->bitmap_left,
		top_edge = max_height - glyph->bitmap_top,
		right_edge = left_edge + bmp->pitch * 8,
		bot_edge = top_edge + bmp->rows;

	if (col < left_edge || col >= right_edge ||
			row < top_edge || row >= bot_edge) {
		return 0;
	}

	const int bmp_x = col - left_edge, bmp_y = row - top_edge;
	unsigned char *row_data;

	if (bmp->pitch < 0) {
		row_data = bmp->buffer + -bmp->pitch * (bmp->rows-1);
	}
	else {
		row_data = bmp->buffer;
	}
	row_data += bmp->pitch * bmp_y;

	return (row_data[bmp_x/8] >> ((7-bmp_x) % 8)) & 1;
}
