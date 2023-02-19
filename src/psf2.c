#include <stdio.h>
#include <stdint.h>

#include <psf2.h>
#include <util.h>
#include <output.h>

struct psf2_header {
	uint8_t magic[4];	/* Always \x72\xb5\x4a\x86 */
	uint32_t version;	/* Always 0 */
	uint32_t header_size;	/* Always sizeof(psf2_header) */
	uint32_t flags;		/* Set to 1 if there's a unicode table */
	uint32_t length;	/* Number of glyphs */
	uint32_t char_size; 	/* (width+7) / 8 * height */
	uint32_t height;	/* Width and height in pixels */
	uint32_t width;
};

#define PSF2_MAGIC0 0x72
#define PSF2_MAGIC1 0xb5
#define PSF2_MAGIC2 0x4a
#define PSF2_MAGIC3 0x86
#define PSF2_HAS_UNICODE_TABLE 0x01
#define PSF2_MAXVERSION 0
#define PSF2_SEPARATOR 0xff
#define PSF2_STARTSEQ 0xfe

static int write_psf2_header(int width, int height, int has_unicode, int glyphs,
		FILE *output);
static int write_psf2_unichar(FT_ULong charcode, FILE *output);
static int write_psf2_startseq(FILE *output);
static int write_psf2_separator(FILE *output);

struct psf_interface psf2_interface = {
	.write_header = write_psf2_header,
	.write_unichar = write_psf2_unichar,
	.write_startseq = write_psf2_startseq,
	.write_separator = write_psf2_separator,
};
/* This is the most satisfying thing I've ever written */

static int write_psf2_header(int width, int height, int has_unicode, int glyphs,
		FILE *output) {
	struct psf2_header header;

	header.magic[0] = PSF2_MAGIC0;
	header.magic[1] = PSF2_MAGIC1;
	header.magic[2] = PSF2_MAGIC2;
	header.magic[3] = PSF2_MAGIC3;
	header.version = 0;
	header.header_size = htop32(sizeof header);
	header.flags = htop32(PSF2_HAS_UNICODE_TABLE);
	header.length = htop32(glyphs);
	header.char_size = htop32((width+7) / 8 * height);
	header.height = htop32(height);
	header.width = htop32(width);

	return fwrite(&header, sizeof header, 1, output) < 1;
}

static int write_psf2_unichar(FT_ULong charcode, FILE *output) {
	if (charcode <= 0x7f) {
		if (fputc(charcode, output) == EOF) {
			return 1;
		}
		return 0;
	}
	unsigned char sequence[4];
	int seqlen = 0;

	memset(sequence, 0x80, sizeof sequence);

	if (charcode <= 0x7ff) {
		sequence[0] = 0xc0;
		seqlen = 2;
	}
	else if (charcode <= 0xffff) {
		sequence[0] = 0xe0;
		seqlen = 3;
	}
	else if (charcode <= 0x10ffff) {
		sequence[0] = 0xf0;
		seqlen = 4;
	}
	else {
		return 1;
	}
	for (int i = seqlen - 1; i >= 0; --i) {
		sequence[i] |= charcode & 0x3f;
		charcode >>= 6;
	}
	if (fwrite(sequence, seqlen, 1, output) < 1) {
		return 1;
	}
	return 0;
}

static int write_psf2_startseq(FILE *output) {
	return fputc(PSF2_STARTSEQ, output) == EOF;
}

static int write_psf2_separator(FILE *output) {
	return fputc(PSF2_SEPARATOR, output) == EOF;
}
