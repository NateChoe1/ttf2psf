#include <stdio.h>
#include <stdint.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <output.h>

struct psf2_header {
	char magic[4];		/* Always \x72\xb5\x4a\x86 */
	uint32_t version;	/* Always 0 */
	uint32_t header_size;	/* Always sizeof(psf2_header) */
	uint32_t flags;		/* Set to 1 if there's a unicode table */
	uint32_t length;	/* Number of glyphs */
	uint32_t char_size; 	/* (width+7) / 8 * height */
	uint32_t height;	/* Width and height in pixels */
	uint32_t width;
};

static uint32_t htop32(uint32_t n); /* host to psf byte order (little-endian) */
static inline int write_glyph(FT_GlyphSlotRec *glyph, FILE *output,
		int width, int height);
static inline int get_bit(FT_GlyphSlotRec *glyph,
		int row, int col, int width, int height);

int write_output(FT_Library library, FT_Face face, FILE *output,
		int width, int height) {
	struct psf2_header header;
	const int glyph_count = 256;
	uint8_t *bitmap;

	memcpy(header.magic, "\x72\xb5\x4a\x86", sizeof header.magic);
	header.version = htop32(0);
	header.header_size = htop32(sizeof header);
	header.flags = htop32(0);
	header.length = htop32(glyph_count);
	header.char_size = htop32((width+7) / 8 * height);
	header.height = htop32(height);
	header.width = htop32(width);

	if (fwrite(&header, sizeof header, 1, output) < 1) {
		fputs("Failed to write PSF header to output\n", stderr);
		return 1;
	}

	bitmap = malloc((width+7) / 8 * height);
	if (bitmap == NULL) {
		fputs("Failed to allocated bitmap\n", stderr);
		return 1;
	}

	for (FT_ULong charcode = 0; charcode < glyph_count; ++charcode) {
		FT_Load_Char(face, charcode,
				FT_LOAD_RENDER | FT_LOAD_MONOCHROME);
		write_glyph(face->glyph, output, width, height);
	}
	return 0;
}

static uint32_t htop32(uint32_t n) {
	uint8_t ret[sizeof n];
	for (int i = 0; i < sizeof n; ++i) {
		ret[i] = n & 0xff;
		n >>= 8;
	}
	return * (uint32_t *) ret;
}

static inline int write_glyph(FT_GlyphSlotRec *glyph, FILE *output,
		int width, int height) {
	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < (width+7) / 8; ++j) {
			unsigned char curr_char;
			curr_char = 0x00;
			for (int k = 0; k < 8; ++k) {
				curr_char <<= 1;
				curr_char |= get_bit(glyph, i, j*8 + k,
						width, height);
			}
			fputc(curr_char, output);
		}
	}
	return 0;
}

static inline int get_bit(FT_GlyphSlotRec *glyph,
		int row, int col, int width, int height) {
	const FT_Bitmap *bmp = &glyph->bitmap;
	const int
		left_edge = glyph->bitmap_left,
		right_edge = glyph->bitmap_left + bmp->width,
		top_edge = height - glyph->bitmap_top,
		bot_edge = top_edge + bmp->rows;
	/* TODO: There may be an off-by-one error in top-edge */

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
