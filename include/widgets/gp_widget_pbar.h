//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGET_PBAR_H
#define GP_WIDGET_PBAR_H

enum gp_widget_pbar_type {
	GP_WIDGET_PBAR_NONE,
	GP_WIDGET_PBAR_PERCENTS,
	GP_WIDGET_PBAR_SECONDS,
	GP_WIDGET_PBAR_TMASK = 0x7f,
	GP_WIDGET_PBAR_INVERSE = 0x80,
};

struct gp_widget_pbar {
	float max;
	float val;
	enum gp_widget_pbar_type type;
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
gp_widget *gp_widget_pbar_new(float val, float max, enum gp_widget_pbar_type type);

/**
 * @brief Sets a progress bar value.
 *
 * @self A progress bar widget.
 * @val New progress bar value.
 */
void gp_widget_pbar_set(gp_widget *self, float val);

/**
 * @brief Sets a progress bar maximum.
 *
 * @self A progress bar widget.
 * @max New maximal progress bar value.
 */
void gp_widget_pbar_set_max(gp_widget *self, float max);

#endif /* GP_WIDGET_PBAR_H */
