// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2023 Cyril Hrubis <metan@ucw.cz>
 */

/*
 * There are a few rebranded driver chips that are mostly the same:
 *
 * UC8179: https://www.buydisplay.com/download/ic/UC8179.pdf
 * GD7965: https://www.waveshare.net/datasheet/EK_PDF/EK79655.pdf
 *
 * This chip is used in these displays:
 *
 * https://www.waveshare.com/w/upload/6/60/7.5inch_e-Paper_V2_Specification.pdf
 */

#ifndef GP_DISPLAY_UC8179_H
#define GP_DISPLAY_UC8179_H

enum uc8179_psr {
	/* Use LUT from register instead of OTP */
	UC8179_PSR_LUT_REG = 0x20,
	/* B/W mode (KW) instead of B/W/R (KWR) */
	UC8179_PSR_KW = 0x10,
	/* Gate scan direction scans up instead of down */
	UC8179_PSR_UD = 0x08,
	/* Source shift direction shift right instead of left */
	UC8179_PSR_SHL = 0x04,
	/* Turn on booster switch */
	UC8179_PSR_SHD_N = 0x02,
	/* Turn off soft reset */
	UC8179_PSR_RST_N = 0x01,
};

/*
 * UC8179 e-ink driver constants
 */
enum uc8179_cmds {
	/* Panel settings */
	UC8179_PSR = 0x00,
	/* Power settings */
	UC8179_PWR = 0x01,
	/* High voltage boosters, controllers, regulators, temperature sensor, power off */
	UC8179_POF = 0x02,
	/* Power off sequence setting */
	UC8179_PFS = 0x03,
	/*
	 * High voltage boosters, controllers, regulators, temperature sensor, power on
	 *
	 * May damage display if it's kept on for a long time!
	 */
	UC8179_PON = 0x04,

	/* Booster soft start */
	UC8179_BSST = 0x06,

	/* Deep sleep mode, can be waken up by reset */
	UC8179_DSLP = 0x07,

	/*
	 * Data start transmission 1
	 *
	 * In B&W mode writes OLD data
	 * IN B&W&R mode writes B&W data
	 */
	UC8179_DTM1 = 0x10,

	/* Display refresh */
	UC8179_DRF = 0x12,

	/*
	 * Data start transmission 2
	 *
	 * In B&W mode writes NEW data
	 * IN B&W&R mode writes RED data
	 */
	UC8179_DTM2 = 0x13,

	/* Dual SPI */
	UC8179_DUSPI = 0x15,

	/* VCOM LUT */
	UC8179_LUTC = 0x20,
	/* Waveform LUTs */
	UC8179_LUTWW = 0x21,
	UC8179_LUTKW = 0x22,
	UC8179_LUTWK = 0x23,
	UC8179_LUTKK = 0x24,
	UC8179_LUTBD = 0x25,
	UC8179_LUTR1 = 0x26,
	UC8179_LUTR2 = 0x27,

	/* Clock frequency */
	UC8179_PLL = 0x30,

	/* VCON settings */
	UC8179_VCON = 0x50,

	/* TCON settings */
	UC8179_TCON = 0x60,

	/* Set display resolution */
	UC8179_TRES = 0x61,

	/* Gate source start settings */
	UC8179_GSST = 0x65,

	/* VCOM_DC voltage */
	UC8179_VDCS = 0x82,

	/*
	 * Sets partial window
	 *
	 * 10 bit integers are send as:
	 *
	 * b0 - - - - - - 9 8
	 * b1 7 6 5 4 3 2 1 0
	 *
	 * Horizontal start & 0x03
	 * Horizontal end   | 0x03
	 *
	 * Vertical start
	 * Vertical end
	 */
	UC8179_PTL = 0x90,
	/* Enter partial mode */
	UC8179_PTIN = 0x91,
	/* Exit partial mode */
	UC8179_PTOUT = 0x92,
};

#endif /* GP_DISPLAY_UC8179_H */
