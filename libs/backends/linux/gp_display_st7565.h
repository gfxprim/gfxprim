// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2023 Erik LHC Hamera <y08192eh@slizka.chapadla.cz>
 * Copyright (C) 2024 Cyril Hrubis <metan@ucw.cz>
 */

/*
 * Constants for ST7565 LCD display driver
 *
 * These displays may differ in size. Maximal size supported by controller is 65x131 pixels.
 */

#ifndef GP_DISPLAY_ST7565_H
#define GP_DISPLAY_ST7565_H

/*
 * ST7565 LCD driver constants
 */
enum st7565_cmds {
	ST7565_ACD_NORMAL  = 0xa0, /* Column address normal */
	ST7565_ACD_INVERSE = 0xa1, /* Column address inverse */

	ST7565_COM_NORMAL  = 0xc0, /* Set COM output direction, normal mode */
	ST7565_COM_INVERSE = 0xc8, /* Set COM output direction, reverse mode */

	ST7565_DISPOFF     = 0xae, /* Display OFF (sleep mode) */
	ST7565_DISPON      = 0xaf, /* Display ON in normal mode */

	ST7565_SETSTARTLINE =        0x40, /* 0x40-7f: Set display start line */
	ST7565_STARTLINE_MASK =      0x3f,

	ST7565_SETPAGESTART =        0xb0, /* 0xb0-b7: Set page start address */
	ST7565_PAGESTART_MASK =      0x07,

	ST7565_SETCOL_L =            0x00, /* 0x00-0x0f: Set lower column address */
	ST7565_COL_L_MASK =          0x0f,
	ST7565_SETCOL_H =            0x10, /* 0x10-0x1f: Set higher column address */
	ST7565_COL_H_MASK =          0x0f,

	ST7565_DISPNORMAL =          0xa6, /* 0xa6: Normal display */
	ST7565_DISPINVERSE =         0xa7, /* 0xa7: Inverse display */

	ST7565_DISP_RAM =            0xa4, /* 0xa4: Resume to RAM content display */
	ST7565_DISP_ENTIRE_ON =      0xa5, /* 0xa5: Entire display ON */

	ST7565_BIAS_1_9 =            0xa2, /* 0xa2: Select BIAS setting 1/9 */
	ST7565_BIAS_1_7 =            0xa3, /* 0xa3: Select BIAS setting 1/7 */

	ST7565_ENTER_RMWMODE =       0xe0, /* 0xe0: Enter the Read Modify Write mode */
	ST7565_EXIT_RMWMODE =        0xee, /* 0xee: Leave the Read Modify Write mode */
	ST7565_EXIT_SOFTRST =        0xe2, /* 0xe2: Software RESET */

	ST7565_POWERCTRL =           0x28, /* Control built-in power circuit, lower three bits are VF=0x01, VR=0x02, VC=0x04 */

	ST7565_RES_RATIO =           0x20, /* Regulation Resistior ratio, lower three bits set the ratio */

	ST7565_CONTRAST =            0x81, /* Set contrast control 1st byte; 2nd byte is 0b000 value */

	ST7565_SETBOOSTER =          0xf8, /* Set booster level 1st byte */
	ST7565_SETBOOSTER_4X =       0x00, /* Set booster level 2nd byte */
	ST7565_SETBOOSTER_5X =       0x01, /* Set booster level 2nd byte */

	ST7565_NOP =                 0xe3,
};

#endif /* GP_DISPLAY_ST7565_H */
