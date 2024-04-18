@ include source.t
/*
 * sRGB correction lookup tables
 *
 * Copyright (C) 2012-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include <stdint.h>

uint16_t gp_srgb8_to_lin10_tbl[256] = {
@ for i in range(0, 256):
@     v = float(i)/255
@     max = 1023
@     if v <= 0.04045:
	{{ int((v/12.92 * max + 0.5)) }}, /* {{ i }} (lin) */
@     else:
	{{ int((((v + 0.055)/1.055) ** 2.4) * max + 0.5) }}, /* {{ i }} */
@ end
};

uint8_t gp_lin10_to_srgb8_tbl[1024] = {
@ for i in range(0, 1024):
@     v = float(i)/1023
@     max = 255
@     if v <= 0.0031308:
	{{ int(12.92 * v * max + 0.5) }}, /* {{ i }} (lin) */
@     else:
	{{ int((1.055 * (v ** (1/2.4)) - 0.055) * max + 0.5) }}, /* {{ i }} */
@ end
};
