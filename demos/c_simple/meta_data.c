// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

 /*

   Read image meta-data and print them into stdout.

  */

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <gfxprim.h>

#define SEP \
"-----------------------------------------------------------------------------"

int main(int argc, char *argv[])
{
	gp_storage *storage;
	int i;

	if (argc < 2) {
		fprintf(stderr, "Takes an image(s) as parameter(s)\n");
		return 1;
	}

	storage = gp_storage_create();

	if (!storage) {
		fprintf(stderr, "Failed to create data storage\n");
		return 1;
	}

	for (i = 1; i < argc; i++) {
		puts(SEP);
		printf("Opening '%s'\n", argv[i]);

		gp_storage_clear(storage);

		if (gp_load_meta_data(argv[i], storage)) {
			fprintf(stderr, "Failed to read '%s' meta-data: %s\n",
			        argv[i], strerror(errno));
		} else {
			gp_storage_print(storage);
		}
	}

	puts(SEP);

	return 0;
}
