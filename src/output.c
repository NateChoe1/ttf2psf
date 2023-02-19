#include <stdio.h>
#include <stdint.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <output.h>
#include <charset.h>

struct glyph *read_face(FT_Face face, int *glyph_count);
static int write_glyph(FT_Face face, int width, int height, FILE *output);
static inline int get_bit(FT_Face face,
		int row, int col, int width, int height);
static int div_up(int num, int denom);

int write_psf(int width, int height, struct psf_interface *interface,
		FT_Face face, FILE *output) {
	struct charset *charset;
	struct glyph *iter;
	FT_Error error;

	charset = new_charset(face);
	if (charset == NULL) {
		return 1;
	}
	for (int i = 0x20; i <= 0x7f; ++i) {
		if (add_char(charset, i)) {
			goto error;
		}
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
	const int
		left_edge = glyph->bitmap_left,
		top_edge = (height-1) - glyph->bitmap_top -
			div_up(face->bbox.yMin, face->units_per_EM),
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

	return (row_data[bmp_x/8] >> ((8-bmp_x) % 8)) & 1;
}

static int div_up(int num, int denom) {
	int ret = num / denom;
	return ret + (num % denom) ? 1:0;
}
