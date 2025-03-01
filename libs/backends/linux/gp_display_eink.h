// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2023-2025 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_display_eink.h
 * @brief Implements asynchronous e-ink display repaint.
 *
 * When display is idle we set up a GPIO as an interrupt source, request
 * repaint and return to the application. When application requests repaints
 * while the display is busy, we queue and merge the area to be repainted and
 * send the request to the display only after the previous repaint finished.
 *
 * The code also manages partial vs full refreshes. Partial refreshes introduce
 * a bit of noise on the screen, so when several partial refreshes are
 * schedulled in a row we have to schedulle a full refres as well to clean up
 * the display.
 */

#ifndef GP_DISPLAY_EINK_H
#define GP_DISPLAY_EINK_H

#include <utils/gp_bbox.h>
#include <utils/gp_timer.h>
#include <backends/gp_backend.h>

#include "gp_display_spi.h"

/**
 * @brief An e-ink display.
 */
struct gp_display_eink {
	/**
	 * @brief display SPI connection.
	 */
	struct gp_display_spi spi;

	/**
	 * @brief Partial refresh counter.
	 */
	unsigned int part_cnt;

	/* how long on average repaint takes */
	unsigned int full_repaint_ms;
	unsigned int part_repaint_ms;

	/* flags */
	unsigned int part_in_progress:1;
	unsigned int full_in_progress:1;
	unsigned int do_full:1;
	unsigned int do_part:1;
	unsigned int exitting:1;

	/**
	 * @brief Accumulated partial refresh area.
	 *
	 * While display is busy any partial refreshes are merged into these coordinates.
	 */
	gp_coord x0, y0, x1, y1;

	/**
	 * @brief File descriptor for the busy edge interrupts.
	 *
	 * This is added to the poll() loop and receives events when edge
	 * interrupts are enabled.
	 */
	gp_fd busy_fd;

	/* callbacks to driver that sends the data/commands */
	void (*repaint_full_start)(gp_backend *self);
	void (*repaint_full_finish)(gp_backend *self);

	void (*repaint_part_start)(gp_backend *self, gp_coord x0, gp_coord y0, gp_coord x1, gp_coord y1);
	void (*repaint_part_finish)(gp_backend *self);

	void (*display_exit)(gp_backend *self);
};

/**
 * @brief Initializes an e-ink display.
 *
 * This is called internally by the e-ink display backends.
 *
 * @brief self An e-ink backend.
 */
void gp_display_eink_init(gp_backend *self);

#endif /* GP_DISPLAY_EINK_H */
