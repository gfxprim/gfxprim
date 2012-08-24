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

#ifndef TST_PRELOAD_FILE_H
#define TST_PRELOAD_FILE_H

enum tst_file_call {
	TST_FAIL_FOPEN  = 0x01,
	TST_FAIL_FCLOSE = 0x02,
};

/*
 * Describes which call on which file and how should fail.
 */
struct tst_fail_FILE {
	/* 
	 * File path to be failed, this is matched exactly against the path
	 * passed to fopen().
	 *
	 * TODO: should we support regexps?
	 */
	const char *path;

	/* pointer to opened file, don't touch */
	FILE *f;

	/* if not zero, errno is set to this */
	int err;
	
	/*	
	 * Which call should be failed.
	 */
	enum tst_file_call call;
};

/*
 * Registers NULL-terminated FILE fail table, NULL == no table.
 */
void tst_fail_FILE_register(struct tst_fail_FILE *self);

#endif /* TST_PRELOAD_FILE_H */
