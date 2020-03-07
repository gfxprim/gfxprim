// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

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
