#include <stdio.h>
#include <stdint.h>

#include <psf1.h>
#include <output.h>

struct psf1_header {
	char magic[2];
	uint8_t mode;
	uint8_t char_size;
};

#define PSF1_MAGIC0 0x36
#define PSF1_MAGIC1 0x04

#define PSF1_MODE512 0x01
#define PSF1_MODEHASTAB 0x02
#define PSF1_MODESEQ 0x04
#define PSF1_MAXMODE 0x05

#define PSF1_SEPARATOR 0xffff
#define PSF1_STARTSEQ 0xfffe
/* These are unused */

int write_psf1(FT_Face face, FILE *output, int height) {
	struct psf1_header header;
	const int glyph_count = 256;
	const int width = 8;

	header.magic[0] = PSF1_MAGIC0;
	header.magic[1] = PSF1_MAGIC1;
	header.mode = 0;
	header.char_size = height;

	if (fwrite(&header, sizeof header, 1, output) < 1) {
		fputs("Failed to write PSF header to output\n", stderr);
		return 1;
	}

	for (FT_ULong charcode = 0; charcode < glyph_count; ++charcode) {
		FT_Load_Char(face, charcode,
				FT_LOAD_RENDER | FT_LOAD_MONOCHROME);
		write_glyph(face, output, width, height);
	}
	return 0;
}
