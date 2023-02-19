#include <stdio.h>
#include <string.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <psf1.h>

static void print_help(char *prog_name);
static inline long min(long a, long b);

int main(int argc, char **argv) {
	int error;
	FT_Library library;
	FT_Face face;
	FILE *output;

	if (argc < 3) {
		print_help(argv[0]);
		return 1;
	}

	error = FT_Init_FreeType(&library);
	if (error) {
		fputs("Error occured during library initialization\n", stderr);
		return 1;
	}

	error = FT_New_Face(library, argv[1], 0, &face);

	if (error == FT_Err_Unknown_File_Format) {
		fputs("Unknown font file format\n", stderr);
		return 1;
	}
	else if (error) {
		fputs("Error while reading the font file\n", stderr);
		return 1;
	}

	int width, height;
	if (argc >= 5 ) {
		width = atoi(argv[3]);
		if (width <= 0) {
			fprintf(stderr, "Invalid width %d\n", width);
			return 1;
		}
		height = atoi(argv[4]);
		if (height <= 0) {
			fprintf(stderr, "Invalid height %d\n", height);
			return 1;
		}
	}
	else {
		width = 8;
		height = 16;
	}

	{
		long pwidth = width * face->units_per_EM /
			(face->bbox.xMax - face->bbox.xMin);
		long pheight = height * face->units_per_EM /
			(face->bbox.yMax - face->bbox.yMin);

		pwidth = width * 3 / 2;
		pheight = height * 3 / 2;

		error = FT_Set_Pixel_Sizes(face, min(pwidth, pheight), 0);
		/* TODO: Get the em box working properly */
	}

	if (error) {
		fprintf(stderr, "Failed to set size %dx%d\n", width, height);
		return 1;
	}

	output = fopen(argv[2], "wb");
	if (output == NULL) {
		fprintf(stderr, "Failed to open file %s for writing\n",
				argv[2]);
		return 1;
	}

	return write_psf1(face, output, height * width / 8);
}

static void print_help(char *prog_name) {
	fprintf(stderr, "Usage: %s [font file] [output] (width) (height)\n",
			prog_name);
}

static inline long min(long a, long b) {
	if (a < b) {
		return a;
	}
	return b;
}
