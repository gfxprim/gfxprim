/*****************************************************************************
 * This file is part of gfxprim library.                                     *
 *                                                                           *
 * Gfxprim is free software; you can redistribute it and/or                  *
 * modify it under the terms of the GNU Lesser General Public                *
 * License as published by the Free Software Foundation; either              *
 * version 2.1 of the License, or (at your option) any later version.        *
 *                                                                           *
 * Gfxprim is distributed in the hope that it will be useful,                *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Lesser General Public License for more details.                           *
 *                                                                           *
 * You should have received a copy of the GNU Lesser General Public          *
 * License along with gfxprim; if not, write to the Free Software            *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,                        *
 * Boston, MA  02110-1301  USA                                               *
 *                                                                           *
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "tst_test.h"

extern const struct tst_suite tst_suite;

/* defined in tst_suite.c */
extern int tst_suite_verbose;

void print_help(void)
{
	fprintf(stderr, "Test suite '%s' Usage:\n\n", tst_suite.suite_name);
	fprintf(stderr, "-h       prints this help\n");
	fprintf(stderr, "-l       list all tests\n");
	fprintf(stderr, "-t name  runs single test by name\n");
	fprintf(stderr, "-v       turns on verbose mode\n");
	fprintf(stderr, "without any option, all tests are executed\n");
}

int main(int argc, char *argv[])
{
	int opt;

	while ((opt = getopt(argc, argv, "hlt:v")) != -1) {
		switch (opt) {
		case 'l':
			tst_list_suite(&tst_suite);
			return 0;
		break;
		case 't':
			tst_run_suite(&tst_suite, optarg);
			return 0;
		break;
		case 'h':
			print_help();
			return 0;
		break;
		case 'v':
			tst_suite_verbose = 1;
		break;
		default:
			print_help();
			return 1;
		}
	}
	
	tst_run_suite(&tst_suite, NULL);

	return 0;
}
