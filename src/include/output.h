#ifndef HAVE_OUTPUT
#define HAVE_OUTPUT

#include <ft2build.h>
#include FT_FREETYPE_H

int write_output(FT_Library library, FT_Face face, FILE *output,
		int width, int height);

#endif
