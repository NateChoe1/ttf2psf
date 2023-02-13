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

static uint32_t htop32(uint32_t n) {
	uint8_t ret[sizeof n];
	for (int i = 0; i < sizeof n; ++i) {
		ret[i] = n & 0xff;
		n >>= 8;
	}
	return * (uint32_t *) ret;
}

static void print_bin(int v, int b);

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
		if (charcode < 'A' || charcode > 'Z')
			continue;
		FT_Load_Char(face, charcode, FT_LOAD_RENDER | FT_LOAD_MONOCHROME);
		for (int i = 0; i < face->glyph->bitmap.rows; ++i) {
			for (int j = 0; j < face->glyph->bitmap.width; ++j) {
				if (face->glyph->bitmap.buffer[i * face->glyph->bitmap.pitch + (j/8)] & (0x80 >> (j % 8))) {
					printf("X");
					printf("X");
				}
				else {
					printf(" ");
					printf(" ");
				}
			}
			putchar('\n');
		}
		putchar('\n');
	}
	return 0;
}

static void print_bin(int v, int b) {
	int new_val = 0;
	for (int i = 0; i < b; ++i) {
		new_val <<= 1;
		new_val |= v & 1;
		v >>= 1;
	}

	for (int i = 0; i < b; ++i) {
		if (new_val & 1)
			printf("XX");
		else
			printf("  ");
		new_val >>= 1;
	}
	putchar('\n');
}
