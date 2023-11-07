// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2023 Cyril Hrubis <metan@ucw.cz>
 */

/*
 * Constants for ssd1677, ssd1680, ssd1681
 *
 * These controllers mostly differ in maximal size of the display they can
 * drive.
 */

#ifndef GP_SSD16XX_H
#define GP_SSD16XX_H

#define SSD16XX_LUT_SIZE 105

/*
 * SSD16XX e-ink driver constants
 */
enum ssd16xx_cmds {
	/*
	 * Driver output control
	 *
	 * First two bytes are last Y coordinate (H-1)
	 *
	 * Third byte sets gate order, default 0x00
	 *
	 * 0x01 - turn on inverse scanning H-1 ... 0
	 * 0x02 - enables even first, odd second
	 * 0x04 - swaps order or gates even first, odd second
	 */
	SSD16XX_DOC = 0x01,

	/*
	 * Booster soft start
	 */
	SSD16XX_BSST = 0x0c,

	/*
	 * Deep sleep mode
	 *
	 * 0x00 - normal mode
	 * 0x03 - deep sleep mode
	 *
	 * Hardware reset is required to exit the deep sleep mode.
	 */
	SSD16XX_DSLP = 0x10,

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
	 * The default is supposed to be 0x03 but the WeeAct 2.13 display has
	 * inverted Y after soft reset. Doh.
	 */
	SSD16XX_DEM = 0x11,

	/*
	 * Software reset
	 *
	 * Sleep for 10ms after reset.
	 */
	SSD16XX_SW_RESET = 0x12,

	/*
	 * Temperature control.
	 *
	 * 0x48 = external
	 * 0x80 = internal
	 */
	SSD16XX_TEMP_CTRL = 0x18,

	/*
	 * Activate update sequence.
	 *
	 * This command triggers the display refresh.
	 *
	 * The busy signal is high during the update.
	 */
	SSD16XX_DISP_UPDT = 0x20,

	/*
	 * Display update control 1
	 */
	SSD16XX_UPDT_CTRL1 = 0x21,

	/*
	 * Display update control 2
	 */
	SSD16XX_UPDT_CTRL2 = 0x22,

	/*
	 * Start write B/W RAM.
	 */
	SSD16XX_WRITE_BW_RAM = 0x24,
	SSD16XX_WRITE_RED_RAM = 0x26,

	/*
	 * Write Register for Display Option
	 */
	SSD16XX_WRDO = 0x37,

	/*
	 * Border waveform control
	 *
	 * How should the border be update if at all.
	 *
	 * Default 0xc0 - HiZ (does not update border)
	 */
	SSD16XX_BWC = 0x3c,

	/*
	 * Set X RAM start/end
	 *
	 * Defines display width.
	 *
	 * Default 0x000 - 0x3bf
	 */
	SSD16XX_XRAM_RANGE = 0x44,

	/*
	 * Set X RAM start/end
	 *
	 * Defines display height.
	 *
	 * Default 0x000 - 0x2a7
	 */
	SSD16XX_YRAM_RANGE = 0x45,

	/* Fills RED RAM with pattern */
	SSD16XX_CLR_RED_RAM = 0x46,
	/* Fills B/W RAM with pattern */
	SSD16XX_CLR_BW_RAM = 0x47,

	/*
	 * Set X RAM address counter initial value.
	 */
	SSD16XX_XRAM_ADDR = 0x4e,

	/*
	 * Set Y RAM address counter initial value.
	 */
	SSD16XX_YRAM_ADDR = 0x4f,
};

#endif /* GP_SSD16XX_H */
