#ifndef HAVE_PSF1
#define HAVE_PSF1

#include <ft2build.h>
#include FT_FREETYPE_H

int write_psf1(FT_Face face, FILE *output, int height);

#endif
