// ttf2psf - A simple, working utility to generate Linux console fonts
// Copyright (C) 2023  Nate Choe
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILIY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include <fcntl.h>
#include <unistd.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <psf1.h>
#include <psf2.h>

static void print_help(char *prog_name);
static inline long min(long a, long b);
static FILE *xfopen(char *filename, char *mode);
static FILE *xgfopen(char *filename);
/* data -> returned file -> gzip -> filename */

int main(int argc, char **argv) {
	int error;
	FT_Library library;
	FT_Face face;
	FILE *output, *charset, *equivalence;
	int width, height;
	struct psf_interface *interface;
	char *charset_name, *equivalence_name;
	int gzip = 0;

	interface = &psf2_interface;
	width = 8;
	height = 16;
	charset_name = equivalence_name = NULL;
	for (;;) {
		int opt = getopt(argc, argv, "12hgw:r:c:e:");
		if (opt < 0) {
			break;
		}
		switch (opt) {
		case '1':
			interface = &psf1_interface;
			break;
		case '2':
			interface = &psf2_interface;
			break;
		case 'h':
			print_help(argv[0]);
			return 0;
		case 'g':
			gzip = 1;
			break;
		case 'w':
			width = atoi(optarg);
			if (width == 0 || height < 0) {
				fprintf(stderr, "Invalid width %s\n", optarg);
				return 1;
			}
			break;
		case 'r':
			height = atoi(optarg);
			if (height == 0 || height < 0) {
				fprintf(stderr, "Invalid height %s\n", optarg);
				return 1;
			}
			break;
		case 'c':
			charset_name = optarg;
			break;
		case 'e':
			equivalence_name = optarg;
			break;
		default:
			print_help(argv[0]);
			return 1;
		}
	}
	if (optind + 2 > argc) {
		fputs("No input/output files specified\n", stderr);
		print_help(argv[0]);
		return 1;
	}
	if (charset_name == NULL) {
		fputs("No charset file specified\n", stderr);
		print_help(argv[0]);
		return 1;
	}
	if (equivalence_name == NULL) {
		fputs("No equivalence file specified\n", stderr);
		print_help(argv[0]);
		return 1;
	}

	if (gzip) {
		output = xgfopen(argv[optind + 1]);
	}
	else {
		output = xfopen(argv[optind + 1], "w");
	}
	charset = xfopen(charset_name, "r");
	equivalence = xfopen(equivalence_name, "r");

	error = FT_Init_FreeType(&library);
	if (error) {
		fputs("Error occured during library initialization\n", stderr);
		return 1;
	}

	error = FT_New_Face(library, argv[optind], 0, &face);

	if (error == FT_Err_Unknown_File_Format) {
		fputs("Unknown font file format\n", stderr);
		return 1;
	}
	else if (error) {
		fputs("Error while reading the font file\n", stderr);
		return 1;
	}

	{
		int px = width * face->units_per_EM / face->max_advance_width;
		error = FT_Set_Pixel_Sizes(face, px, px);
	}

	if (error) {
		fprintf(stderr, "Failed to set size %dx%d\n", width, height);
		return 1;
	}

	return write_psf(width, height, interface, charset, equivalence, face,
			output);
}

static void print_help(char *prog_name) {
	fprintf(stderr,
"Usage: %s (-1) (-2) (-g) (-h) (-w [char width]) (-r [char height])\n"
"               -c [char set] -e [equivalence file]\n"
"               [input font] [output font.psfu]\n"
"    -1     : Output a psf1 file\n"
"    -2     : Output a psf2 file (default)\n"
"    -g     : Output a gzip compressed file\n"
"    -h     : Show this help menu\n"
"    -w, -r : Sets character width and row count respectively (default: 8x16)\n"
"    -c     : Specify a character set (see /usr/share/ttf2psf/charsets\n"
"    -e     : Specify an equivalence file (see /usr/share/ttf2psf/equivalence\n"
			,
			prog_name);
}

static inline long min(long a, long b) {
	if (a < b) {
		return a;
	}
	return b;
}

static FILE *xfopen(char *filename, char *mode) {
	FILE *ret = fopen(filename, mode);
	if (ret == NULL) {
		fprintf(stderr, "Failed to open file %s\n", filename);
		exit(EXIT_FAILURE);
	}
	return ret;
}

static FILE *xgfopen(char *filename) {
#define CLOSE(fd) { if (close(fd) < 0) { perror("close() failed"); } }
#define DUP2(old, new) { if (dup2(old, new) < 0) { perror("dup2() failed"); } }
	int fd[2], outfd;
	pid_t pid;
	FILE *ret;
	if (pipe(fd) < 0) {
		perror("Failed to open pipe");
		exit(EXIT_FAILURE);
	}
	pid = fork();
	switch (pid) {
	case -1:
		perror("fork() failed");
		exit(EXIT_FAILURE);
	case 0:
		outfd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
		if (outfd < 0) {
			perror("open() failed");
			exit(EXIT_FAILURE);
		}
		CLOSE(fd[1]);
		DUP2(fd[0], 0);
		DUP2(outfd, 1);
		execlp("gzip", "gzip", "-c", NULL);
		perror("execlp() failed");
		exit(EXIT_FAILURE);
	default:
		CLOSE(fd[0]);
		ret = fdopen(fd[1], "w");
		if (ret == NULL) {
			fputs("fdopen() failed\n", stderr);
			exit(EXIT_FAILURE);
		}
		return ret;
	}
#undef CLOSE
#undef DUP2
}
