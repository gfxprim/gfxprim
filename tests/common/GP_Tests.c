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
 * Copyright (C) 2011 Tomas Gavenciak <gavento@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <check.h>

typedef Suite* (SuiteFactory)(void);

/* 
 * Declare all the manualy created Suite-generating functions here:
 */

SuiteFactory* manual_suites[] = {
	NULL	/* Sentinel */
};

/*
 * Generated function creating and adding all the suites
 */

void GP_AddSuitesToSRunner(SRunner *sr);


const char usage[] = "Usage:\n%s [-v] [-q]\n";

int main(int argc, char *argv[])
{
	int verb = CK_NORMAL;

	int opt;	
	while((opt = getopt(argc, argv, "vq")) >= 0) 
		switch(opt) {
			case 'v': 
				verb = CK_VERBOSE;
				break;
			case 'q':
				verb = CK_SILENT;
				break;
			default:
				fprintf(stderr, usage, argv[0]);
				return(EXIT_FAILURE);
		}

	SRunner *sr = srunner_create(NULL);

	SuiteFactory **s;
	for (s = manual_suites; *s; s++) {
		srunner_add_suite(sr, (*s)());
	}
	GP_AddSuitesToSRunner(sr);
	
	srunner_run_all(sr, verb);
	int number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
