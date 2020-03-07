// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

/*

  Example on custom debug message handler.

 */

#include <stdio.h>
#include <gfxprim.h>

static char level_to_c(int level)
{
	switch (level) {
	case GP_DEBUG_TODO:
		return 'T';
	case GP_DEBUG_WARN:
		return 'W';
	case GP_DEBUG_BUG:
		return 'B';
	case GP_DEBUG_FATAL:
		return 'F';
	case 0 ... 9:
		return '0' + level;
	default:
		return 'U';
	}
}

void debug_handler(const struct gp_debug_msg *msg)
{
	printf("%c: %s->%s():%u: %s\n", level_to_c(msg->level), msg->file,
	       msg->fn, msg->line, msg->msg);
}

int main(void)
{
	/* Set custom debug handler */
	gp_set_debug_handler(debug_handler);

	/* Print some debug messages */
	GP_WARN("This is a warning");
	GP_FATAL("This is a fatal condition");

	/* Turn on verbose debug and call some library functions */
	gp_set_debug_level(10);

	gp_pixmap *pixmap = gp_pixmap_alloc(1000, 1000, 1);

	gp_filter_gaussian_blur(pixmap, pixmap, 10, 10, NULL);

	gp_pixmap_free(pixmap);

	return 0;
}
