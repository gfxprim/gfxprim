// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include <stdarg.h>
#include <errno.h>

#include <core/gp_version.h>
#include <core/gp_common.h>
#include <core/gp_debug.h>

static unsigned int debug_level = GP_DEFAULT_DEBUG_LEVEL;

static int env_used = 0;

static void (*debug_handler)(const struct gp_debug_msg *msg) = NULL;

void gp_set_debug_level(unsigned int level)
{
	debug_level = level;
}

unsigned int gp_get_debug_level(void)
{
	return debug_level;
}

void gp_set_debug_handler(void (*handler)(const struct gp_debug_msg *msg))
{
	debug_handler = handler;
}

void gp_debug_print(int level, const char *file, const char *function, int line,
                    const char *fmt, ...)
{
	int i, err;

	err = errno;

	if (!env_used) {
		char *level = getenv("GP_DEBUG");

		env_used = 1;

		if (level != NULL) {
			int new_level = atoi(level);

			if (new_level >= 0) {
				debug_level = new_level;

				GP_DEBUG(1, "Using debug level GP_DEBUG=%i "
				             "from enviroment variable",
					     debug_level);
			}
		}

		GP_DEBUG(1, "GFXprim library version " GP_VER_STR);
	}

	if (level > (int)debug_level)
		goto end;

	/* If handler is set, fill struct msg and call it */
	if (debug_handler) {
		char buf[256];

		va_list va;
		va_start(va, fmt);
		vsnprintf(buf, sizeof(buf), fmt, va);
		va_end(va);

		struct gp_debug_msg msg = {
			.level = level,
			.file = file,
			.fn = function,
			.line = line,
			.msg = buf,
		};

		debug_handler(&msg);

		goto end;
	}

	for (i = 1; i < level; i++)
		fputc(' ', stderr);

	switch (level) {
	case GP_DEBUG_FATAL:
		gp_debug_print_cstack();
		fprintf(stderr, "*** FATAL: %s:%s():%u: ", file, function, line);
	break;
	case GP_DEBUG_BUG:
		gp_debug_print_cstack();
		fprintf(stderr, "*** BUG: %s:%s():%u: ", file, function, line);
	break;
	case GP_DEBUG_WARN:
		gp_debug_print_cstack();
		fprintf(stderr, "*** WARNING: %s:%s():%u: ", file, function, line);
	break;
	case GP_DEBUG_TODO:
		fprintf(stderr, "*** TODO: %s:%s():%u: ", file, function, line);
	break;
	default:
		fprintf(stderr, "%u: %s:%s():%u: ",
		        level, file, function, line);
	break;
	}

        va_list va;
	va_start(va, fmt);
	vfprintf(stderr, fmt, va);
	va_end(va);

	fputc('\n', stderr);
end:
	errno = err;
}
