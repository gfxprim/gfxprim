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

#define _GNU_SOURCE
#include <stdio.h>
#include <dlfcn.h>
#include <errno.h>
#include <string.h>

#include "tst_test.h"
#include "tst_preload_FILE.h"

struct tst_fail_FILE *failures = NULL;

void tst_fail_FILE_register(struct tst_fail_FILE *self)
{
	failures = self;
}

static struct tst_fail_FILE *failure_by_path(const char *path,
                                             enum tst_file_call call)
{
	unsigned int i;

	if (failures == NULL)
		return NULL;

	for (i = 0; failures[i].path != NULL; i++)
		if (failures[i].call == call && 
		    !strcmp(path, failures[i].path))
			return &failures[i];

	return NULL;
}

void failures_init_FILE(const char *path, FILE *f)
{
	unsigned int i;

	if (failures == NULL)
		return;

	//TODO: warn on f not NULL
	for (i = 0; failures[i].path != NULL; i++)
		if (!strcmp(path, failures[i].path))
			failures[i].f = f;
}

static struct tst_fail_FILE *failure_by_FILE(FILE *f, enum tst_file_call call)
{
	unsigned int i;

	if (failures == NULL)
		return NULL;

	for (i = 0; failures[i].path != NULL; i++)
		if (failures[i].call == call && f == failures[i].f)
			return &failures[i];

	return NULL;
}

FILE *fopen(const char *path, const char *mode)
{
	static FILE *(*real_fopen)(const char *, const char *);

	if (!real_fopen)
		real_fopen = dlsym(RTLD_NEXT, "fopen");

	struct tst_fail_FILE *failure = failure_by_path(path, TST_FAIL_FOPEN);

	if (failure) {
		if (failure->err)
			errno = failure->err;

		return NULL;
	}


	FILE *f = real_fopen(path, mode);

	failures_init_FILE(path, f);

	return f;
}

int fclose(FILE *fp)
{
	static int (*real_fclose)(FILE *);

	if (!real_fclose)
		real_fclose = dlsym(RTLD_NEXT, "fclose");

	struct tst_fail_FILE *failure = failure_by_FILE(fp, TST_FAIL_FCLOSE);

	/*
	 * We close the file here correctly, we can because when fclose() has
	 * failed any further access results in undefined behavior.
	 */
	if (failure) {
		real_fclose(fp);

		if (failure->err)
			errno = failure->err;

		return EOF;
	}

	return real_fclose(fp);
}
