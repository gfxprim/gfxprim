//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2023 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @file gp_widget_pbar.h
 * @brief A progress bar widget.
 *
 * Progressbar is a widget that shows a progress. The progress is internally
 * stored as a 64-bit integer number that starts at 0 and ends at `max`.
 *
 * The value can be associated with a unit currently `none`, `percents` and
 * `seconds` are supported.
 *
 * Progressbar can also be set to be inverse which means that the value used
 * for unit display will be `max - val` instead of `val`. The default unit is
 * `percents`.
 *
 * If progressbar has an event handler set it's seekable, i.e. the user can
 * change it's value and the `on_event` handler is called for each value
 * change.
 *
 * Progress bar widget JSON attributes
 * -----------------------------------
 *
 * |   Attribute    |  Type  |   Default  |    Description    |
 * |----------------|--------|------------|-------------------|
 * |    **val**     |  uint  |      0     | Initial value     |
 * |    **max**     |  uint  |    10000   | Maximal value     |
 * |  **inverse**   |  bool  |    false   | Inverse progress  |
 * |   **unit**     | string | "percents" | Value units       |
 */
#ifndef GP_WIDGET_PBAR_H
#define GP_WIDGET_PBAR_H

#include <stdint.h>

/** @brief A progress bar widget units. */
enum gp_widget_pbar_unit {
	/**
	 * @brief No unit.
	 *
	 * @image html widget_pbar_none.png
	 */
	GP_WIDGET_PBAR_NONE,
	/**
	 * @brief Progress bar is in percents.
	 *
	 * @image html widget_pbar_percents.png
	 *
	 * The percentage shown to user is computed as 100 * val / max.
	 */
	GP_WIDGET_PBAR_PERCENTS,
	/**
	 * @brief Progress bar is in seconds.
	 *
	 * @image html widget_pbar_seconds.png
	 *
	 * Widget automatically converts the value into hours, minutes and
	 * seconds when shown on the screen.
	 */
	GP_WIDGET_PBAR_SECONDS,
	/** @brief Unit mask. */
	GP_WIDGET_PBAR_UNIT_MASK = 0x7f,
	/**
	 * @brief Inverse the value shown on the screen.
	 *
	 * When set the progress starts at max and counts down towards zero as
	 * val grows.
	 */
	GP_WIDGET_PBAR_INVERSE = 0x80,
};

/** @brief A gp_widget_event::sub_type for a progress bar widget. */
enum gp_widget_pbar_event_type {
	/**
	 * @brief Emitted when progress bar value is changed.
	 *
	 * E.g. music player can have a seekable progress showing the time left
	 * in a song.
	 */
	GP_WIDGET_PBAR_VAL_CHANGED,
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
 * @param val Initial progress bar value.
 * @param max Maximal progress bar value.
 * @param unit Progress bar value unit.
 *
 * @return A progress bar widget.
 */
gp_widget *gp_widget_pbar_new(uint64_t val, uint64_t max,
                              enum gp_widget_pbar_unit unit);

/**
 * @brief Sets a progress bar value.
 *
 * @param self A progress bar widget.
 * @param val New progress bar value.
 */
void gp_widget_pbar_val_set(gp_widget *self, uint64_t val);

/**
 * @brief Sets a progress bar maximum.
 *
 * @param self A progress bar widget.
 * @param max New maximal progress bar value.
 */
void gp_widget_pbar_max_set(gp_widget *self, uint64_t max);

/**
 * @brief Returns progressbar value.
 *
 * @param self A progress bar widget.
 *
 * @return A progress bar value.
 */
uint64_t gp_widget_pbar_val_get(gp_widget *self);

/**
 * @brief Returns progress bar max value.
 *
 * @param self A progress bar widget.
 *
 * @return A progress bar max value.
 */
uint64_t gp_widget_pbar_max_get(gp_widget *self);

#endif /* GP_WIDGET_PBAR_H */
