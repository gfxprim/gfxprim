// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2023 Cyril Hrubis <metan@ucw.cz>
 */

/*
 * 960x680 e-ink display.
 *
 */

#ifndef GP_SSD1677_H
#define GP_SSD1677_H

#define SSD1677_LUT_SIZE 105

/*
 * SSD1677 e-ink driver constants
 */
enum ssd1677_cmds {
	/*
	 * Driver output control
	 */
	SSD1677_DOC = 0x01,

	/*
	 * Booster soft start
	 */
	SSD1677_BSST = 0x0c,

	/*
	 * Deep sleep mode
	 *
	 * 0x00 - normal mode
	 * 0x03 - deep sleep mode
	 *
	 * Hardware reset is required to exit the deep sleep mode.
	 */
	SSD1677_DSLP = 0x10,

	/*
	 * Data entry mode
	 *
	 * How RAM address is updated on data write.
	 *
	 * x  x  x  x  x a2 a1 a0
	 *
	 * a2 = 1 increase Y first
	 * a2 = 0 increase X first
	 *
	 * a1 = 1 inc Y
	 * a1 = 0 dec Y
	 * a0 = 1 inc X
	 * a0 = 0 dec X
	 *
	 * Default 0x03
	 */
	SSD1677_DEM = 0x12,

	/*
	 * Software reset
	 *
	 * Sleep for 10ms after reset.
	 */
	SSD1677_SW_RESET = 0x12,

	/*
	 * Temperature control.
	 *
	 * 0x48 = external
	 * 0x80 = internal
	 */
	SSD1677_TEMP_CTRL = 0x18,

	/*
	 * Activate update sequence.
	 *
	 * This command triggers the display refresh.
	 *
	 * The busy signal is high during the update.
	 */
	SSD1677_DISP_UPDT = 0x20,

	/*
	 * Display update control 1
	 */
	SSD1677_UPDT_CTRL1 = 0x21,

	/*
	 * Display update control 2
	 */
	SSD1677_UPDT_CTRL2 = 0x22,

	/*
	 * Start write B/W RAM.
	 */
	SSD1677_WRITE_BW_RAM = 0x24,
	SSD1677_WRITE_RED_RAM = 0x26,

	/*
	 * Write Register for Display Option
	 */
	SSD1677_WRDO = 0x37,

	/*
	 * Border waveform control
	 *
	 * How should the border be update if at all.
	 *
	 * Default 0xc0 - HiZ (does not update border)
	 */
	SSD1677_BWC = 0x3c,

	/*
	 * Set X RAM start/end
	 *
	 * Defines display width.
	 *
	 * Default 0x000 - 0x3bf
	 */
	SSD1677_XRAM_RANGE = 0x44,

	/*
	 * Set X RAM start/end
	 *
	 * Defines display height.
	 *
	 * Default 0x000 - 0x2a7
	 */
	SSD1677_YRAM_RANGE = 0x45,

	/* Fills RED RAM with pattern */
	SSD1677_CLR_RED_RAM = 0x46,
	/* Fills B/W RAM with pattern */
	SSD1677_CLR_BW_RAM = 0x47,

	/*
	 * Set X RAM address counter initial value.
	 */
	SSD1677_XRAM_ADDR = 0x4e,

	/*
	 * Set Y RAM address counter initial value.
	 */
	SSD1677_YRAM_ADDR = 0x4f,
};

#endif /* GP_SDP1656_H */
