// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2023 Cyril Hrubis <metan@ucw.cz>
 */

/*
 * https://www.waveshare.com/w/upload/6/60/7.5inch_e-Paper_V2_Specification.pdf
 */

#ifndef GP_SPD1656_H
#define GP_SPD1656_H

/*
 * SPD1656 e-ink driver constants
 */
enum spd1656_cmds {
	/* Panel settings */
	SPD1656_PSR = 0x00,
	/* Power settings */
	SPD1656_PWR = 0x01,
	/* High voltage boosters, controllers, regulators, temperature sensor, power off */
	SPD1656_POF = 0x02,
	/* Power off sequence setting */
	SPD1656_PFS = 0x03,
	/*
	 * High voltage boosters, controllers, regulators, temperature sensor, power on
	 *
	 * May damage display if it's kept on for a long time!
	 */
	SPD1656_PON = 0x04,

	/* Booster soft start */
	SPD1656_BSST = 0x06,

	/* Deep sleep mode, can be waken up by reset */
	SPD1656_DSLP = 0x07,

	/*
	 * Data start transmission 1
	 *
	 * In B&W mode writes OLD data
	 * IN B&W&R mode writes B&W data
	 */
	SPD1656_DTM1 = 0x10,

	/* Display refresh */
	SPD1656_DRF = 0x12,

	/*
	 * Data start transmission 2
	 *
	 * In B&W mode writes NEW data
	 * IN B&W&R mode writes RED data
	 */
	SPD1656_DTM2 = 0x13,

	/* Dual SPI */
	SPD1656_DUSPI = 0x15,

	/* VCOM LUT */
	SPD1656_LUTC = 0x20,
	/* Waveform LUTs */
	SPD1656_LUTB = 0x21,
	SPD1656_LUTW = 0x22,
	SPD1656_LUTG1 = 0x23,
	SPD1656_LUTG2 = 0x24,
	SPD1656_LUTR0 = 0x25,
	SPD1656_LUTR1 = 0x26,
	SPD1656_LUTR2 = 0x27,

	/* Clock frequency */
	SPD1656_PLL = 0x30,

	/* VCON settings */
	SPD1656_VCON = 0x50,

	/* TCON settings */
	SPD1656_TCON = 0x60,

	/* Set display resolution */
	SPD1656_TRES = 0x61,

	/* Gate source start settings */
	SPD1656_GSST = 0x65,

	/* VCOM_DC voltage */
	SPD1656_VDCS = 0x82,

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
	SPD1656_PTL = 0x90,
	/* Enter partial mode */
	SPD1656_PTIN = 0x91,
	/* Exit partial mode */
	SPD1656_PTOUT = 0x92,
};

#endif /* GP_SDP1656_H__ */
