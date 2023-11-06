// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2023 Cyril Hrubis <metan@ucw.cz>
 */

#include <core/gp_debug.h>
#include "gp_display_eink.h"

static int can_start_repaint(gp_backend *backend)
{
	struct gp_display_eink *eink = GP_BACKEND_PRIV(backend);

	return !(eink->full_in_progress || eink->part_in_progress);
}

static void gp_display_eink_flip(gp_backend *self)
{
	struct gp_display_eink *eink = GP_BACKEND_PRIV(self);

	if (can_start_repaint(self)) {
		GP_DEBUG(4, "Starting full repaint");
		eink->repaint_full_start(self);
		eink->full_in_progress = 1;
		eink->part_cnt = 0;
		return;
	}

	GP_DEBUG(4, "Queueing full repaint");

	eink->do_full = 1;
	eink->do_part = 0;
}

static void gp_display_eink_update_rect(gp_backend *self, gp_coord x0, gp_coord y0, gp_coord x1, gp_coord y1)
{
	struct gp_display_eink *eink = GP_BACKEND_PRIV(self);

	if (eink->part_cnt >= 5) {
		eink->part_cnt = 0;
		GP_DEBUG(4, "Five partial repaints in row, requesting full repaint");
		gp_display_eink_flip(self);
		return;
	}

	if (can_start_repaint(self)) {
		GP_DEBUG(4, "Starting partial repaint");
		eink->part_cnt++;
		eink->repaint_part_start(self, x0, y0, x1, y1);
		eink->part_in_progress = 1;
		return;
	}

	if (eink->do_full) {
		GP_DEBUG(4, "Full repaint already queued");
		return;
	}

	if (eink->do_part) {
		eink->x0 = GP_MIN(eink->x0, x0);
		eink->y0 = GP_MIN(eink->y0, y0);
		eink->x1 = GP_MAX(eink->x1, x1);
		eink->y1 = GP_MAX(eink->y1, y1);
		GP_DEBUG(4, "Merging partial repaints");
	} else {
		eink->x0 = x0;
		eink->y0 = y0;
		eink->x1 = x1;
		eink->y1 = y1;
		eink->do_part = 1;
		GP_DEBUG(4, "Queueing partial repaint");
	}
}

static int flush_queued_repaints(gp_fd *self)
{
	gp_backend *backend = self->priv;
	struct gp_display_eink *eink = GP_BACKEND_PRIV(backend);

	if (eink->full_in_progress) {
		GP_DEBUG(4, "Finishing full repaint");
		eink->full_in_progress = 0;
		eink->repaint_full_finish(backend);
	}

	if (eink->part_in_progress) {
		GP_DEBUG(4, "Finishing partial repaint");
		eink->part_in_progress = 0;
		eink->repaint_part_finish(backend);
	}

	if (eink->do_full) {
		GP_DEBUG(4, "Starting queued full repaint");
		eink->do_full = 0;
		eink->repaint_full_start(backend);
		eink->full_in_progress = 1;
		return 0;
	}

	if (eink->do_part) {
		GP_DEBUG(4, "Starting queued partial repaint");
		eink->do_part = 0;
		eink->part_cnt++;
		eink->repaint_part_start(backend, eink->x0, eink->y0, eink->x1, eink->y1);
		eink->part_in_progress = 1;
		return 0;
	}

	GP_DEBUG(4, "No repaint queued");

	return 0;
}

void gp_display_eink_init(gp_backend *self)
{
	struct gp_display_eink *eink = GP_BACKEND_PRIV(self);

	eink->part_cnt = 0;

	self->flip = gp_display_eink_flip;
	self->update_rect = gp_display_eink_update_rect;

	eink->busy_fd = (gp_fd) {
		.fd = eink->spi.gpio_map->busy.fd,
		.event = flush_queued_repaints,
		.events = GP_POLLPRI,
		.priv = self,
	};
}
