//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2023 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGET_PBAR_H
#define GP_WIDGET_PBAR_H

#include <stdint.h>

enum gp_widget_pbar_unit {
	GP_WIDGET_PBAR_NONE,
	GP_WIDGET_PBAR_PERCENTS,
	GP_WIDGET_PBAR_SECONDS,
	GP_WIDGET_PBAR_TMASK = 0x7f,
	GP_WIDGET_PBAR_INVERSE = 0x80,
};

struct gp_widget_pbar {
	uint64_t max;
	uint64_t val;
	int32_t step;
	enum gp_widget_pbar_unit unit;
};

/**
 * @brief Allocate and initialize new progress bar widget.
 *
 * @val Initial progress bar value.
 * @max Maximal progress bar value.
 * @type Progress bar value type.
 *
 * @return A progress bar widget.
 */
gp_widget *gp_widget_pbar_new(uint64_t val, uint64_t max,
                              enum gp_widget_pbar_unit unit);

/**
 * @brief Sets a progress bar value.
 *
 * @self A progress bar widget.
 * @val New progress bar value.
 */
void gp_widget_pbar_val_set(gp_widget *self, uint64_t val);

/**
 * @brief Sets a progress bar maximum.
 *
 * @self A progress bar widget.
 * @max New maximal progress bar value.
 */
void gp_widget_pbar_max_set(gp_widget *self, uint64_t max);

/**
 * @brief Returns progressbar value.
 *
 * @self A progress bar widget.
 * @return A progress bar value.
 */
uint64_t gp_widget_pbar_val_get(gp_widget *self);

#endif /* GP_WIDGET_PBAR_H */
