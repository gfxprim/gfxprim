// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2023 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <inttypes.h>

#include <core/gp_core.h>
#include <backends/gp_backend.h>
#include <backends/gp_display.h>

const struct gp_backend_display_model gp_backend_display_models[] = {
	{.name = "WaveShare-7.5-v2",
	 .desc = "1BPP Grayscale 7.5 inch SPI e-ink display"},
	{.name = "WaveShare-3.7",
	 .desc = "1BPP/4BPP Grayscale 3.7 inch SPI e-ink display"},
	{.name = "WeeAct-2.13",
	 .desc = "1BPP Grayscale 2.13 inch SPI e-ink display"},
	{}
};

gp_backend *gp_waveshare_7_5_v2_init(void);
gp_backend *gp_waveshare_3_7_init(void);
gp_backend *gp_weeact_2_13_init(void);

static gp_ev_queue event_queue;

gp_backend *gp_backend_display_init(enum gp_backend_display_model_ids model)
{
	gp_backend *ret = NULL;

	switch (model) {
	case GP_WAVESHARE_7_5_V2:
		ret = gp_waveshare_7_5_v2_init();
	break;
	case GP_WAVESHARE_3_7:
		ret = gp_waveshare_3_7_init();
	break;
	case GP_WEEACT_2_13_BW:
		ret = gp_weeact_2_13_init();
	break;
	default:
		GP_FATAL("Invalid model %i\n", model);
		return NULL;
	}

	if (ret) {
		ret->event_queue = &event_queue;
		gp_ev_queue_init(ret->event_queue, ret->pixmap->w, ret->pixmap->h, 0, GP_EVENT_QUEUE_LOAD_KEYMAP);
	}

	return ret;
}
