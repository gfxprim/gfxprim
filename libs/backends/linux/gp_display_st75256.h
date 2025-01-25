// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2025 Cyril Hrubis <metan@ucw.cz>
 */

/*
 * Constants for ST75256 LCD display driver
 */

#ifndef GP_DISPLAY_ST75256_H
#define GP_DISPLAY_ST75256_H

/*
 * ST75256 LCD driver constants
 *
 * The commands are split into extensions, proper extension has to be selected.
 */
enum st7565_cmds {
	/* Sets Extension1 commands. */
	ST75256_EXT1        = 0x30,
	/* Sets Extension2 commands. */
	ST75256_EXT2        = 0x31,
	/* Sets Extension3 commands. */
	ST75256_EXT3        = 0x38,
	/* Sets Extension4 commands. */
	ST75256_EXT4        = 0x39,

	/* Extension1 commands */
	/**
	 * Power controll, followed by a byte.
	 *
	 * bit or of:
	 * 0x01 VR Internal regulator on/off
	 * 0x02 VF Internal follower on/off
	 * 0x08 VB Internal booster on/off
	 */
	ST75256_PWR_CTRL    = 0x20,

	/**
	 * Sets accessible y part of the DDRAM
	 *
	 * It's followed by two bytes.
	 *
	 * 0x00 - 0x28 min y addr
	 * 0x00 - 0x28 max y addr
	 */
	ST75256_SET_PAGE_ADDR = 0x75,

	/**
	 * Sets accessibble x part of the DDRAM
	 *
	 * It's followed by two bytes.
	 *
	 * 0x00 - 0xff min x addr
	 * 0x00 - 0xff max x addr
	 */
	ST75256_SET_COL_ADDR = 0x15,

	/**
	 * Starts writing continous flow of data to DDRAM.
	 */
	ST75256_WRITE_DATA = 0x5c,

	/**
	 * Sets direction DDRAM is scanned when data are displayed.
	 *
	 * Followed by a byte where bits:
	 *
	 * 0x01 Inverses Y direction.
	 * 0x02 Inversed X direction.
	 * 0x04 Sets page direction first otherwise column direction first.
	 */
	ST75256_SET_SCAN_DIRECTION = 0xbc,

	ST75256_SLEEP_OUT   = 0x94, /* Power state sleep out */
	ST75256_SLEEP_IN    = 0x95, /* Power state sleep in */

	/**
	 * Normal display colors 0 = white.
	 */
	ST75256_DISP_NORM   = 0xa6,

	/**
	 * Inverse display colors 0 == black.
	 */
	ST75256_DISP_INV    = 0xa7,

	ST75256_DISP_OFF    = 0xae, /* Display OFF (sleep mode) */
	ST75256_DISP_ON     = 0xaf, /* Display ON in normal mode */

	/**
	 * Display controll, followed by three bytes.
	 *
	 * 0x00 or 0x04 External clock divider
	 * 0x00 - 0xff Duty cycle.
	 * 0x00 - 0x8f
	 */
	ST75256_DISP_CTRL   = 0xca,

	/**
	 * Set LCD Operating Voltage i.e. Contrast. Followed by two bytes.
	 *
	 * 0x00 - 0x3f Upper bytes of the contrast.
	 * 0x00 - 0x07 Lower bytes of the contrast.
	 */
	ST75256_SET_VOP     = 0x81,

	/* Extension2 commands */

	/**
	 * Sets gray levels, followed by 16 bytes.
	 *
         * The dark and light value is between 0x00 and 0x1f.
	 *
	 * Yes, the dark and light gray bytes are repeated randomly.
	 *
	 * 0x00
	 * 0x00
	 * 0x00
	 * light_gray
	 * light_gray
	 * light_gray
	 * 0x00
	 * 0x00
	 * dark_gray
	 * 0x00
	 * 0x00
	 * dark_gray
	 * dark_gray
	 * dark_gray
	 * 0x00
	 * 0x00
	 */
	ST75256_SET_GRAY_LEVEL = 0x20,

	/**
	 * Sets analog circuts, followed by three data bytes.
	 *
	 * 0x00
	 * 0x00-0x03 Booster efficiency
	 * 0x00-0x07 Bias ratio
	 */
	ST75256_ANALOG_CIRCUIT_SET = 0x32,

	/**
	 * Enables/disables OTP auto read.
	 *
	 * Followed by a single byte.
	 *
	 * 0x8f - enable
	 * 0x9f - disable
	 */
	ST75256_OTP_AUTO_READ_CTRL = 0xd7,

	/**
	 * Enables OTP read/write control.
	 *
	 * Followed by single byte.
	 *
	 * 0x00 enable read
	 * 0x20 enable write
	 */
	ST75256_OTP_WR_RD_CTRL = 0xe0,

	/**
	 * Exits OTP mode.
	 */
	ST75256_OTP_CTRL_OUT = 0xe1,


	/**
	 * Triggers OTP programming.
	 */
	ST75256_OTP_READ = 0xe3,
};

#endif /* GP_DISPLAY_ST75256_H */
