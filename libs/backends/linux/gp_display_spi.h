// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2023 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef GP_DISPLAY_SPI_H
#define GP_DISPLAY_SPI_H

#include <stdint.h>
#include "gp_linux_spi.h"
#include "gp_linux_gpio.h"

struct gp_gpio_map {
	union {
		struct gp_gpio gpio[4];
		struct {
			struct gp_gpio reset;
			struct gp_gpio dc;
			struct gp_gpio pwr;
			struct gp_gpio busy;
		};
	};
};

struct gp_display_spi {
	int spi_fd;
	struct gp_gpio_map *gpio_map;
};

int gp_display_spi_init(struct gp_display_spi *self,
                        const char *spi_dev, uint8_t mode, uint32_t speed,
			struct gp_gpio_map *map);

void gp_display_spi_exit(struct gp_display_spi *self);

void gp_display_spi_cmd(struct gp_display_spi *self, uint8_t cmd);

void gp_display_spi_data(struct gp_display_spi *self, uint8_t data);

int gp_display_spi_busy(struct gp_display_spi *self);

void gp_display_spi_wait_ready(struct gp_display_spi *self, int ready);

#endif /* GP_DISPLAY_SPI_H */
