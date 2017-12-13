@ include source.t
/*
 * Gamma correction tables for Gamma = 2.2
 *
 * Copyright (C) 2012-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include <stdint.h>

/*
 * Converts 8 bit gamma to 10 bit linear.
 */
static uint16_t gamma8_linear10[] = {
@ for i in range(0, 256):
	{{ int(((float(i)/255) ** 2.2) * 1024 + 0.5) }}, /* {{i}} */
@ end
};

/*
 * Converts 10 bit linear to 8 bit gamma.
 */
static uint8_t linear10_gamma8[] = {
@ for i in range(0, 1025):
	{{ int(((float(i)/1024) ** (1/2.2)) * 255 + 0.5) }}, /* {{i}} */
@ end
};

/*
 * Pointers to tables
 */
uint16_t *gp_gamma8_linear10 = gamma8_linear10;
uint8_t *gp_linear10_gamma8 = linear10_gamma8;
