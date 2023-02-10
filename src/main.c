#include <stdio.h>
#include <string.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <output.h>

static void print_help(char *prog_name);

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

	output = fopen(argv[2], "wb");
	if (output == NULL) {
		fprintf(stderr, "Failed to open file %s for writing\n",
				argv[2]);
		return 1;
	}

	error = FT_Set_Pixel_Sizes(face, 8, 16);

	return write_output(library, face, output);
}

static void print_help(char *prog_name) {
	fprintf(stderr, "Usage: %s [font file] [output]\n", prog_name);
}
