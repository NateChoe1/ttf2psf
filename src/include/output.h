#ifndef HAVE_OUTPUT
#define HAVE_OUTPUT

#include <stdio.h>

#include <ft2build.h>
#include FT_FREETYPE_H

struct psf_interface {
	int (*write_header)(int width, int height,
			int has_unicode, int glyphs, FILE *output);
	int (*write_unichar)(FT_ULong charcode, FILE *output);
	int (*write_startseq)(FILE *output);
	int (*write_separator)(FILE *output);
};

int write_psf(int width, int height, struct psf_interface *interface,
		FT_Face face, FILE *output);

#endif
