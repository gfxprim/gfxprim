// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2023 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_display.h
 * @brief Hardware displays.
 *
 * Hardware display backends support various displays connected to hardware
 * GPIO e.g. SPI e-ink RPi Hat. These displays are coupled with Linux input
 * driver so that you can get a computer with a keyboard, mouse and display
 * just by connecting display to GPIOs as well as USB keyboard and/or mouse.
 *
 * In order to start an application with a hardware display backend
 * display:$display_name string has to be passed to the display init function.
 * This is usually done on a commandline by passing the -b option.
 *
 * @attention In order to run display backends as a regular user the user has
 *            to be in the right groups that is gpio and for SPI displays spi
 *            group as well.
 *
 * SPI displays need spidev userspace SPI driver and a few GPIOs for additional
 * signaling. The default RPi GPIO mapping is compatible with WaveShare e-ink
 * display hats.
 *
 * @attention SPI displays require spidev (userspace SPI interface) driver
 *            that is bind to the hardware SPI driver. On RPi this is done by
 *            raspi-config where you enable SPI and reboot. After the reboot
 *            the /dev/spidev0.0 should be available.
 *
 * RPi GPIO Map
 *
 * | Display  | GPIO | Display SPI | GPIO |
 * |----------|------|-------------|------|
 * |  Reset   |  17  |     MOSI    |  10  |
 * | Data/Cmd |  25  |     MISO    |   9  |
 * |  Power   |  18  |     SCLK    |  11  |
 * |  Bussy   |  24  |     CS0     |   8  |
 */
#ifndef BACKENDS_GP_DISPLAY_H
#define BACKENDS_GP_DISPLAY_H

#include <backends/gp_backend.h>

/** @brief Supported displays */
enum gp_backend_display_model_ids {
	/**
	 * @brief WaveShare RPi e-ink HAT 1BPP grayscale 800x480
	 *
	 * Chip GD7965 aka UC8179c
	 */
	GP_WAVESHARE_7_5_V2 = 0,
	/**
	 * @brief WaveShare RPi e-ink HAT 1BPP/4BPP grayscale 280x480
	 *
	 * Chip SSD1677
	 */
	GP_WAVESHARE_3_7 = 1,
	/**
	 * @brief WeeAct 2.13 inch BW display
	 *
	 * Chip SSD1680
	 */
	GP_WEACT_2_13_BW = 2,
	/**
	 * @brief An 128 x 64 BW LCD display.
	 *
	 * 1BPP Grayscale 58.5x46.5mm SPI LCD display.
	 *
	 * Chip ST7565
	 */
	GP_GMG12864 = 3,
	/**
	 * @brief A 256x128 LCD display 1bpp mode
	 *
	 * Chip ST75256
	 */
	GP_JLX256128_1 = 4,
	/**
	 * @brief A 256x128 LCD display 2bpp mode
	 *
	 * Chip ST75256
	 */
	GP_JLX256128_2 = 5,
};

/** @brief A display model description.  */
struct gp_backend_display_model {
	/** @brief A human readable display name used as id. */
	const char *name;
	/** @brief A display description. */
	const char *desc;
};

/**
 * @brief NULL-terminated array of display model names indexed by
 *        enum #gp_backend_display_model_ids.
 */
extern const struct gp_backend_display_model gp_backend_display_models[];

/**
 * @brief Initializes a hardware display.
 *
 * @warning Using wrong display model or connecting it wrongly can damage your
 *          hardware!
 *
 * The backend uses Linux input devices for keyboard and mouse support.
 *
 * @param model A display model.
 * @return An initialized backend.
 */
gp_backend *gp_backend_display_init(enum gp_backend_display_model_ids model);

#endif /* BACKENDS_GP_DISPLAY_H */
