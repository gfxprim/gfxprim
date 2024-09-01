//SPDX-License-Identifier: LGPL-2.0-or-later
/*

   Copyright (c) 2014-2024 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @file gp_widget_log.h
 * @brief A text log widget.
 *
 * Log widget JSON attributes
 * --------------------------
 *
 * |   Attribute    |  Type  | Default | Description                                                      |
 * |----------------|--------|---------|------------------------------------------------------------------|
 * |  **max_logs**  |  uint  |         | A maximal number of log lines the widget can store.              |
 * |  **min_lines** |  uint  |         | A minimal number of log lines visible on the screen.             |
 * |  **min_width** |  uint  |         | A minimal widget width in text characters.                       |
 * |    **tattr**   | string | normal  | Log text attributes #gp_widget_tattr, e.g. `mono` for monospace. |
 */

#ifndef GP_WIDGET_LOG_H
#define GP_WIDGET_LOG_H

#include <stdlib.h>
#include <utils/gp_cbuffer.h>

/**
 * @brief Appends text to the log.
 *
 * @param self A log widget.
 * @param text Text to be appended.
 */
void gp_widget_log_append(gp_widget *self, const char *text);

/**
 * @brief Allocates a log widget.
 *
 * The log widget is a circular buffer of loglines, when size reaches the
 * max_log_lines the oldest message gets removed from the log.
 *
 * @param tattr Text attributes
 * @param min_width Minimal width in letters
 * @param min_lines Minimal number of lines
 * @param max_logs Maximal number of log lines stored internally.
 *
 * @return Newly allocated log widget.
 */
gp_widget *gp_widget_log_new(gp_widget_tattr tattr,
                             unsigned int min_width, unsigned int min_lines,
			     size_t max_logs);

#endif /* GP_WIDGET_LOG_H */
