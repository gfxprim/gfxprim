// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2023 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <inttypes.h>

#include <core/gp_core.h>
#include <backends/gp_backend.h>
#include <backends/gp_display.h>

#include <backends/gp_display_st77xx.h>

const struct gp_backend_display_model gp_backend_display_models[] = {
	[GP_WAVESHARE_7_5_V2] = {.name = "WaveShare-7.5-v2",
	                         .desc = "1BPP Grayscale 7.5 inch SPI e-ink display"},
	[GP_WAVESHARE_3_7] = {.name = "WaveShare-3.7",
	                      .desc = "1BPP/4BPP Grayscale 3.7 inch SPI e-ink display"},
	[GP_WEACT_2_13_BW] = {.name = "WeAct-2.13",
	                      .desc = "1BPP Grayscale 2.13 inch SPI e-ink display"},
	[GP_WEACT_2_9_BW] = {.name = "WeAct-2.9",
	                      .desc = "1BPP Grayscale 2.9 inch SPI e-ink display"},
	[GP_GMG12864] = {.name = "GMG12864",
	                 .desc = "1BPP Grayscale 58.5x46.5mm SPI LCD display"},
	[GP_JLX256128_1] = {.name = "JLX256128-1",
	                    .desc = "JLX Grayscale SPI LCD display 1bpp mode"},
	[GP_JLX256128_2] = {.name = "JLX256128-2",
	                    .desc = "JLX Grayscale SPI LCD display 2bpp mode"},
	[GP_ST7789_1_9] = {.name = "ST7789-1.9",
	                   .desc = "TFT 1.9\" 170x320 ST7789 LCD display"},
	[GP_GMT020_02] = {.name = "GMT020-02",
	                   .desc = "TFT 2.0\" 240x320 ST7789 LCD display"},
	[GP_ST7789_2_8] = {.name = "ST7789-2.8",
	                   .desc = "TFT 2.8\" 240x320 ST7789 LCD display"},
	[GP_ST7796_3_5] = {.name = "ST7796-3.5",
	                   .desc = "TFT 3.5\" 320x480 ST7796 LCD display"},
	[GP_ST7735_1_8] = {.name = "ST7735-1.8",
	                   .desc = "TFT 1.8\" 128x160 ST7735 LCD display"},
	{}
};

gp_backend *gp_waveshare_7_5_v2_init(void);
gp_backend *gp_waveshare_3_7_init(void);
gp_backend *gp_weact_2_13_init(void);

gp_backend *gp_ssd16xx_init(unsigned int w, unsigned int h, int dpi);

gp_backend *gp_st7565_init(unsigned int dpi);
gp_backend *gp_st75256_init(unsigned int dpi, unsigned int bpp);

/*
 * Horizontal DPI is 108
 * Vertical DPI is 100
 */
#define JLX256128_DPI 104

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
	case GP_WEACT_2_13_BW:
		ret = gp_ssd16xx_init(122, 250, 130);
	break;
	case GP_WEACT_2_9_BW:
		ret = gp_ssd16xx_init(128, 296, 125);
	break;
	case GP_GMG12864:
		ret = gp_st7565_init(67);
	break;
	case GP_JLX256128_1:
		ret = gp_st75256_init(JLX256128_DPI, 1);
	break;
	case GP_JLX256128_2:
		ret = gp_st75256_init(JLX256128_DPI, 2);
	break;
	case GP_ST7789_1_9:
		ret = gp_display_st77xx_init(170, 320, 35, 0, 184, GP_DISPLAY_ST77XX_INV);
	break;
	case GP_GMT020_02:
		ret = gp_display_st77xx_init(240, 320, 0, 0, 199, GP_DISPLAY_ST77XX_INV);
	break;
	case GP_ST7789_2_8:
		ret = gp_display_st77xx_init(240, 320, 0, 0, 141, 0);
	break;
	case GP_ST7796_3_5:
		ret = gp_display_st77xx_init(320, 480, 0, 0, 166,
		                             GP_DISPLAY_ST77XX_BGR |
		                             GP_DISPLAY_ST77XX_MIRROR_X);
	break;
	case GP_ST7735_1_8:
		ret = gp_display_st77xx_init(128, 160, 0, 0, 115, 0);
	break;
	default:
		GP_FATAL("Invalid model %i\n", model);
		return NULL;
	}

	if (ret) {
		ret->event_queue = &event_queue;
		gp_ev_queue_init(ret->event_queue, ret->pixmap->w, ret->pixmap->h, 0, NULL, NULL, GP_EVENT_QUEUE_LOAD_KEYMAP);
	}

	return ret;
}
