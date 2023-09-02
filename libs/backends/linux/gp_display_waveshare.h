// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2023 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef GP_DISPLAY_WAVESHARE_H
#define GP_DISPLAY_WAVESHARE_H

#include "gp_display_spi.h"

#define EINK_SPI_DEV "/dev/spidev0.0"
#define EINK_SPI_SPEED_HZ 10000000
/**
 * on SPI_MODE_0
 * off SPI_CS_HIGH
 * off SPI_LSB_FIRST
 * off SPI_3WIRE
 * off SPI_LOOP
 * off SPI_NO_CS
 * off SPI_READY
 */
#define EINK_SPI_MODE (SPI_MODE_0)

/*
 * The chip select GPIO 8 is handled by the kernel SPI driver
 */
/* WARNING: if you mess up direction you may fry the chip! */
static struct gp_gpio_map gpio_map_rpi = {
	.reset  = {.nr = 17, .dir = GP_GPIO_OUT},
	.dc     = {.nr = 25, .dir = GP_GPIO_OUT},
	.pwr    = {.nr = 18, .dir = GP_GPIO_OUT},
	.busy   = {.nr = 24, .dir = GP_GPIO_IN},
};

#endif /* GP_DISPLAY_WAVEHARE_H */
