#include <stdio.h>
#include <stdint.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <util.h>
#include <output.h>

static inline int get_bit(FT_Face face,
		int row, int col, int width, int height);
static int div_up(int num, int denom);

int write_glyph(FT_Face face, FILE *output, int width, int height) {
	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < (width+7) / 8; ++j) {
			unsigned char curr_char;
			curr_char = 0x00;
			for (int k = 0; k < 8; ++k) {
				curr_char <<= 1;
				curr_char |= get_bit(face,
						i, j*8 + k, width, height);
			}
			fputc(curr_char, output);
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
