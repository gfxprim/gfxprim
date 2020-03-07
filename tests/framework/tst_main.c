// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "tst_test.h"

extern const struct tst_suite tst_suite;

/* defined in tst_suite.c */
extern int tst_suite_verbose;
extern const char *tst_log_dir;

void print_help(void)
{
	fprintf(stderr, "Test suite '%s' Usage:\n\n", tst_suite.suite_name);
	fprintf(stderr, "-h       prints this help\n");
	fprintf(stderr, "-l       list all tests\n");
	fprintf(stderr, "-t name  runs single test by name\n");
	fprintf(stderr, "-v       turns on verbose mode\n");
	fprintf(stderr, "-o       test log output dir\n");
	fprintf(stderr, "without any option, all tests are executed\n");
}

int main(int argc, char *argv[])
{
	int opt;

	while ((opt = getopt(argc, argv, "hlo:t:v")) != -1) {
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
		case 'o':
			tst_log_dir = optarg;
		break;
		default:
			print_help();
			return 1;
		}
	}

	tst_run_suite(&tst_suite, NULL);

	return 0;
}
