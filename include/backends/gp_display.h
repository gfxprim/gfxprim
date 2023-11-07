// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2023 Cyril Hrubis <metan@ucw.cz>
 */

/*
 * Hardware displays.
 */

#ifndef BACKENDS_GP_DISPLAY_H
#define BACKENDS_GP_DISPLAY_H

#include <backends/gp_backend.h>

enum gp_backend_display_model_ids {
	/* WaveShare RPi e-ink HAT 1BPP grayscale 800x480 */
	GP_WAVESHARE_7_5_V2 = 0,
	/* WaveShare RPi e-ink HAT 1BPP/4BPP grayscale 280x480 */
	GP_WAVESHARE_3_7 = 1,
	/* WeeAct 2.13 inch BW display */
	GP_WEEACT_2_13_BW = 2,
};

struct gp_backend_display_model {
	const char *name;
	const char *desc;
};

/*
 * NULL-terminated array of display model names
 */
extern const struct gp_backend_display_model gp_backend_display_models[];

gp_backend *gp_backend_display_init(enum gp_backend_display_model_ids model);

#endif /* BACKENDS_GP_DISPLAY_H */
