#include <stdio.h>
#include <stdint.h>

#include <psf1.h>
#include <output.h>

struct psf1_header {
	uint8_t magic[2];
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

static int write_psf1_header(int width, int height, int has_unicode, int glyphs,
		FILE *output);
static int write_psf1_unichar(FT_ULong charcode, FILE *output);
static int write_psf1_startseq(FILE *output);
static int write_psf1_separator(FILE *output);

struct psf_interface psf1_interface = {
	.write_header = write_psf1_header,
	.write_unichar = write_psf1_unichar,
	.write_startseq = write_psf1_startseq,
	.write_separator = write_psf1_separator,
};
/* This is the most satisfying thing I've ever written */

static int write_psf1_header(int width, int height, int has_unicode, int glyphs,
		FILE *output) {
	struct psf1_header header;
	if ((glyphs != 256 && glyphs != 512) || width != 8) {
		return 1;
	}
	header.magic[0] = PSF1_MAGIC0;
	header.magic[1] = PSF1_MAGIC1;
	header.mode = PSF1_MODEHASTAB;
	if (glyphs == 512) {
		header.mode |= PSF1_MODE512;
	}
	header.char_size = height;
	if (fwrite(&header, sizeof header, 1, output) < 1) {
		return 1;
	}
	return 0;
}

static int write_psf1_unichar(FT_ULong charcode, FILE *output) {
	if (charcode > 0xffff) {
		return 1;
	}
	if (fputc(charcode & 0xff, output) == EOF) {
		return 1;
	}
	charcode >>= 8;
	if (fputc(charcode & 0xff, output) == EOF) {
		return 1;
	}
	return 0;
}

static int write_psf1_startseq(FILE *output) {
	const uint8_t startseq[] = { 0xff, 0xfe };
	return fwrite(startseq, sizeof startseq, 1, output) < 1;
}

static int write_psf1_separator(FILE *output) {
	const uint8_t separator[] = { 0xff, 0xff };
	return fwrite(separator, sizeof separator, 1, output) < 1;
}
