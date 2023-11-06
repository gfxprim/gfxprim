// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2023 Cyril Hrubis <metan@ucw.cz>
 */

/*
 * Implements asynchronous e-ink display repaint. Instead of waiting for
 * display busy signal we return to the application and set up a timer for an
 * estimate when the refresh should be done. While the display is busy the area
 * of display that should be repainted is recorded.
 *
 * Once the timer callback is called we check if there is anything to repaint,
 * if not we turn off the display power, otherwise we repaint the display and
 * start the timer again.
 *
 * If the area to be repainted is small, we do partial refresh, if large full
 * refresh. Also after a few partial refreshes the display needs a full refresh
 * to get rid of the accumulated noise.
 */

#ifndef GP_DISPLAY_EINK_H
#define GP_DISPLAY_EINK_H

#include <utils/gp_bbox.h>
#include <utils/gp_timer.h>
#include <backends/gp_backend.h>

#include "gp_display_spi.h"

struct gp_display_eink {
	/* display hardware connection */
	struct gp_display_spi spi;

	/* partial refresh counter for e-paper */
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

	gp_coord x0, y0, x1, y1;

	/* File descriptor for the busy interrupts */
	gp_fd busy_fd;

	/* callbacks to driver that sends the data/commands */
	void (*repaint_full_start)(gp_backend *self);
	void (*repaint_full_finish)(gp_backend *self);

	void (*repaint_part_start)(gp_backend *self, gp_coord x0, gp_coord y0, gp_coord x1, gp_coord y1);
	void (*repaint_part_finish)(gp_backend *self);

	void (*display_exit)(gp_backend *self);
};

void gp_display_eink_init(gp_backend *self);

#endif /* GP_DISPLAY_EINK_H */
