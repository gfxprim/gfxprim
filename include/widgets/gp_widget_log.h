//SPDX-License-Identifier: LGPL-2.0-or-later
/*

   Copyright (c) 2014-2023 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGET_LOG_H
#define GP_WIDGET_LOG_H

#include <stdlib.h>
#include <utils/gp_cbuffer.h>

struct gp_widget_log {
	gp_widget_tattr tattr;
	unsigned int min_width;
	unsigned int min_lines;

	gp_cbuffer log;
	char **logs;
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
 * The log widget is a circular buffer of loglines, when size reaches the
 * max_log_lines the oldest message gets removed from the log.
 *
 * @tattr Text attributes
 * @min_width Minimal width in letters
 * @min_lines Minimal number of lines
 * @max_logs Maximal number of log lines stored internally.
 *
 * @return Newly allocated log widget.
 */
gp_widget *gp_widget_log_new(gp_widget_tattr tattr,
                             unsigned int min_width, unsigned int min_lines,
			     size_t max_logs);

#endif /* GP_WIDGET_LOG_H */
