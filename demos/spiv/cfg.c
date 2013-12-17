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
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include <core/GP_Debug.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
#include <getopt.h>

#include "cfg.h"

static int terminal(struct cfg_opt *opt)
{
	return opt->key == NULL && opt->opt == 0 && opt->opt_long == NULL;
}

struct cfg_opt *opt_by_key(struct cfg_opt *cfg_opts, const char *name_space,
                           const char *key)
{
	struct cfg_opt *i;

	GP_DEBUG(1, "Looking for key '%s' in [%s]", key, name_space);

	for (i = cfg_opts; !terminal(i); i++) {
		if (!i->key)
			continue;

		if (name_space && !i->name_space)
			continue;

		if (name_space && strcmp(i->name_space, name_space))
			continue;

		if (!strcmp(key, i->key))
			break;
	}

	if (terminal(i))
		return NULL;

	GP_DEBUG(1, "Found key '%s' in [%s]", i->key, i->name_space);

	return i;
}

int cfg_getopt(struct cfg_opt *opts, int argc, char *argv[])
{
	unsigned int count = 0;
	unsigned int long_count = 0;
	struct cfg_opt *i;
	int c, opt_idx;

	for (i = opts; !terminal(i); i++) {
		if (i->opt) {
			count++;

			if (i->opt_has_value)
				count++;
		}

		if (i->opt_long)
			long_count++;
	}

	char gopt[count+1];
	struct option gopt_long[long_count+1];
	struct cfg_opt *gopts_long[long_count+1];
	struct cfg_opt *gopts[256];

	memset(gopt_long, 0, sizeof(gopt_long));
	memset(gopts, 0, sizeof(gopts));

	count = 0;
	long_count = 0;

	for (i = opts; !terminal(i); i++) {
		if (i->opt) {
			gopt[count++] = i->opt;

			if (i->opt_has_value)
				gopt[count++] = ':';

			if ((unsigned)i->opt >= sizeof(gopts)/(sizeof(*gopts))) {
				GP_WARN("Char value > 256???");
			} else {
				gopts[(unsigned)i->opt] = i;
			}
		}

		if (i->opt_long) {
			gopt_long[long_count].name = i->opt_long;
			gopt_long[long_count].has_arg = i->opt_has_value;
			gopt_long[long_count].flag = NULL;
			gopt_long[long_count].val = 0;
			gopts_long[long_count] = i;
			long_count++;
		}
	}

	gopt[count] = 0;

	GP_DEBUG(1, "Have getopt string '%s' and %u long opts",
	         gopt, long_count);

	for (;;) {
		c = getopt_long(argc, argv, gopt, gopt_long, &opt_idx);

		switch (c) {
		case -1:
			return optind;
		case 0:
			GP_DEBUG(2, "Long option '%s'",
			         gopts_long[opt_idx]->opt_long);

			i = gopts_long[opt_idx];
			i->val = optarg;

			if (i->set(i, 0))
				return -1;
		break;
		default:
			GP_DEBUG(2, "Short option '%c'", c);

			if ((unsigned)((char)c) >= sizeof(gopts)/(sizeof(*gopts))) {
				GP_WARN("Getopt returned %i???", c);
				break;
			}

			i = gopts[(unsigned)((char)c)];

			if (!i)
				return -1;

			i->val = optarg;

			if (i->set(i, 0))
				return -1;
		break;
		}

	}
}

struct parser_state {
	struct cfg_opt *opts;
	unsigned int lineno;
	FILE *f;
	char name_space[128];
	char buf[1024];
};

static void parser_error(struct parser_state *state, const char *fmt, ...)
                        __attribute__ ((format (printf, 2, 3)));

static void parser_error(struct parser_state *state, const char *fmt, ...)
{
	va_list va;

	fprintf(stderr, "ERROR: %u: ", state->lineno);
	va_start(va, fmt);
	vfprintf(stderr, fmt, va);
	va_end(va);
	fprintf(stderr, "\n");
}

static void read_comment(struct parser_state *state)
{
	int c;

	for (;;) {
		c = getc_unlocked(state->f);

		switch (c) {
		case EOF:
			return;
		case '\n':
			state->lineno++;
			return;
		default:
		break;
		}
	}
}

static int parse_namespace(struct parser_state *state)
{
	size_t len = 0;
	int c;

	for (;;) {
		c = getc_unlocked(state->f);

		switch (c) {
		case EOF:
			parser_error(state,
			             "End of file while parsing namespace");
			return 1;
		case '\n':
			parser_error(state, "Newline while parsing namespace");
			return 1;
		case ']':
			state->name_space[len] = '\0';
			GP_DEBUG(1, "In namespace [%s]", state->name_space);
			return 0;
		default:
			state->name_space[len++] = c;

			if (len >= sizeof(state->name_space)) {
				parser_error(state, "Namespace too long");
				return 1;
			}
		break;
		}
	}
}

static int read_key(struct parser_state *state)
{
	size_t len = 0;
	int c;

	for (;;) {
		c = getc_unlocked(state->f);

		switch (c) {
		case '\n':
			state->lineno++;
		case EOF:
		case ' ':
		case '\t':
		case '=':
			ungetc(c, state->f);
			state->buf[len] = '\0';
			GP_DEBUG(1, "Have key '%s'", state->buf);
			return 0;
		default:
			state->buf[len++] = c;

			if (len >= sizeof(state->buf)) {
				state->buf[sizeof(state->buf) - 1] = '\0';
				parser_error(state, "Key '%s...' is too long",
				             state->buf);
				return 1;
			}
		break;
		}
	}
}

static void read_whitespaces(struct parser_state *state)
{
	int c;

	for (;;) {
		c = getc_unlocked(state->f);

		switch (c) {
		case '\n':
			state->lineno++;
		case ' ':
		case '\t':
		break;
		case EOF:
			return;
		default:
			ungetc(c, state->f);
			return;
		}
	}
}

static const char *name_space(struct parser_state *state)
{
	if (state->name_space[0])
		return state->name_space;

	return NULL;
}

static int parse_pair(struct parser_state *state)
{
	struct cfg_opt *opt;
	int c;

	if (read_key(state))
		return 1;

	opt = opt_by_key(state->opts, name_space(state), state->buf);

	if (!opt) {
		parser_error(state, "There is no key '%s' in name space '%s'",
		             state->buf, name_space(state));
		return 1;
	}

	if (!opt->opt_has_value) {
		if (opt->set(opt, state->lineno))
			return 1;

		return 0;
	}

	read_whitespaces(state);

	c = fgetc_unlocked(state->f);

	switch (c) {
	case EOF:
		parser_error(state, "End of file while looking for =");
		return 1;
	case '=':
	break;
	default:
		parser_error(state, "Expected = got '%c'", c);
	}

	read_whitespaces(state);

	//TODO write read val with quotation marks and poker
	if (read_key(state))
		return 1;

	opt->val = state->buf;

	if (opt->set(opt, state->lineno))
		return 1;

	return 0;
}

static int parse_cfg(struct parser_state *state)
{
	int c;

	for (;;) {
		c = getc_unlocked(state->f);

		switch (c) {
		case EOF:
			GP_DEBUG(1, "End of config reached at line %u",
			         state->lineno);
			return 0;
		case '\n':
			state->lineno++;
		case ' ':
		case '\t':
		break;
		case '#':
			read_comment(state);
		break;
		case '[':
			if (parse_namespace(state))
				return 1;
		break;
		default:
			ungetc(c, state->f);
			if (parse_pair(state))
				return 1;
		break;
		}
	}
}

int cfg_load(struct cfg_opt *opts, const char *path)
{
	struct parser_state state;
	int ret;

	state.opts = opts;
	state.lineno = 1;
	state.f = fopen(path, "r");
	state.name_space[0] = '\0';

	if (!state.f) {
		GP_WARN("Failed to open '%s': %s'", path, strerror(errno));
		return 1;
	}

	ret = parse_cfg(&state);
	fclose(state.f);

	return ret;
}

const char *has_value(struct cfg_opt *opt)
{
	if (opt->opt_has_value)
		return "=value";

	return "";
}

void cfg_print_help(struct cfg_opt *opts)
{
	struct cfg_opt *i;
	const char *name_space = NULL;

	printf("\n");

	for (i = opts; !terminal(i); i++) {
		if ((name_space && !i->name_space) ||
		    (name_space && strcmp(i->name_space, name_space)) ||
                    (!name_space && i->name_space)) {

			printf("\n");

			if (i->name_space)
				printf(" %s:\n", i->name_space);

			name_space = i->name_space;
		}

		if (i->opt && !i->opt_long)
			printf("  -%c%s\n", i->opt, has_value(i));

		if (!i->opt && i->opt_long)
			printf("  --%s%s\n", i->opt_long, has_value(i));

		if (i->opt && i->opt_long) {
			printf("  -%c%s, --%s%s\n", i->opt, has_value(i),
			       i->opt_long, has_value(i));
		}

		if (i->help)
			printf("  \t%s\n\n", i->help);
	}
}

static void man_escape_print(const char *str)
{
	while (*str) {
		switch (*str) {
		case '-':
			printf("\\-");
		break;
		default:
			printf("%c", *str);
		break;
		}
		str++;
	}
}

void cfg_print_man(struct cfg_opt *opts)
{
	struct cfg_opt *i;
	const char *name_space = NULL;

	printf(".SH OPTIONS\n");

	for (i = opts; !terminal(i); i++) {
		if ((name_space && !i->name_space) ||
		    (name_space && strcmp(i->name_space, name_space)) ||
                    (!name_space && i->name_space)) {

			if (i->name_space)
				printf(".TP\n.I %s\n", i->name_space);

			name_space = i->name_space;
		}

		if (i->opt || i->opt_long)
			printf(".TP\n");

		if (i->opt && !i->opt_long)
			printf(".B  \\-%c%s\n", i->opt, has_value(i));

		if (!i->opt && i->opt_long)
			printf(".B  \\-\\-");

		if (i->opt && i->opt_long)
			printf(".B  \\-%c%s, \\-\\-", i->opt, has_value(i));

		if (i->opt_long) {
			man_escape_print(i->opt_long);
			printf("%s\n", has_value(i));
		}

		if (i->help) {
			man_escape_print(i->help);
			printf("\n");
		}
	}

	printf(".SH CONFIGURATION FILE\n");

	printf("Configuraton file has simple key = value syntax, ");
	printf("keys without values are written just as key.\n");
	printf("Lines started with # are comments.\n");
	printf("Keys are grouped in namespaces, namespace block is started\n");
	printf("by [NameSpace] and continues until next namespace.\n");

	name_space = NULL;

	for (i = opts; !terminal(i); i++) {
		if ((name_space && !i->name_space) ||
		    (name_space && strcmp(i->name_space, name_space)) ||
                    (!name_space && i->name_space)) {
			if (i->name_space)
				printf(".TP\n.I [%s]\n", i->name_space);

		    name_space = i->name_space;
		}
		if (i->key) {
			printf(".TP\n.B %s%s\n", i->key, has_value(i));
			if (i->help)
				printf("%s\n", i->help);
		}
	}
}
