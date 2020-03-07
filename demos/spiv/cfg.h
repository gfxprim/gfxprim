// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

 /*

   Configuration loader/storage.

   Configuration is simple namespace,key -> value storage, the configuration
   could either be loaded from config file and/or overriden by argumant parser.

  */

#ifndef __CFG_H__
#define __CFG_H__

struct cfg_opt {
	/* Could be NULL for global values */
	const char *name_space;
	/* Must be set */
	const char *key;
	/* Short command line option, i.e. -f */
	const char opt;
	/* Long command option, i.e. --foo */
	const char *opt_long;
	/* set to 1 if option has parameter */
	int opt_has_value;
	/* help string */
	const char *help;
	/* setter function, called for each parset key = val pair */
	int (*set)(struct cfg_opt *self, unsigned int lineno);
	/* pointer to pass value from parser */
	const char *val;
};

/*
 * Loads configuration from a file.
 *
 * Returns zero on success, non-zero on failure (and prints error message into
 * stderr).
 */
int cfg_load(struct cfg_opt *opts, const char *path);

/*
 * Parses configuration options from command line parameters.
 *
 * Returns number of used strings from argv on success, -1 on failure.
 */
int cfg_getopt(struct cfg_opt *opts, int argc, char *argv[]);

/*
 * Prints help for switches.
 */
void cfg_print_help(struct cfg_opt *opts);

/*
 * Prints man-page formatted options + config.
 */
void cfg_print_man(struct cfg_opt *opts);

#endif /* __CFG_H__ */
