//SPDX-License-Identifier: LGPL-2.0-or-later
/*

   Copyright (c) 2014-2022 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGET_LOG_H
#define GP_WIDGET_LOG_H

#include <stdlib.h>

struct gp_widget_log {
	gp_widget_tattr tattr;
	unsigned int min_width;
	unsigned int min_lines;
	const char **logs;
};

/**
 * @brief Appends text to the log.
 *
 * @self A log widget.
 * @param text Text to be appended.
 */
void gp_widget_log_append(gp_widget *self, const char *text);

/**
 * @brief Allocates a log widget.
 *
 * @return Newly allocated log widget.
 */
gp_widget *gp_widget_log_new(gp_widget_tattr tattr,
                             unsigned int min_width, unsigned int min_lines);

#endif /* GP_WIDGET_LOG_H */
