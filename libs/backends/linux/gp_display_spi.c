// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2023-2025 Cyril Hrubis <metan@ucw.cz>
 */

#include <core/gp_common.h>
#include <core/gp_debug.h>

#include "gp_linux_gpio.h"
#include "gp_linux_spi.h"
#include "gp_display_spi.h"

int gp_display_spi_init(struct gp_display_spi *self,
                        const char *spi_dev, uint8_t mode, uint32_t speed,
			struct gp_gpio_map *map, uint16_t w, uint16_t h)
{
	int ret;

	self->spi_fd = gp_spi_open(spi_dev, mode, speed);

	if (self->spi_fd < 0)
		return 1;

	ret = gp_gpio_export(map->gpio, GP_ARRAY_SIZE(map->gpio), "SPI display");
	if (ret) {
		gp_spi_close(self->spi_fd);
		return 1;
	}

	self->gpio_map = map;

	self->w = w;
	self->h = h;

	return 0;
}

void gp_display_spi_exit(struct gp_display_spi *self)
{
	gp_spi_close(self->spi_fd);

	gp_gpio_write(&self->gpio_map->pwr, 0);
	gp_gpio_write(&self->gpio_map->dc, 0);
	gp_gpio_write(&self->gpio_map->reset, 0);

	gp_gpio_unexport(self->gpio_map->gpio, GP_ARRAY_SIZE(self->gpio_map->gpio));
}

void gp_display_spi_cmd(struct gp_display_spi *self, uint8_t cmd)
{
	gp_gpio_write(&self->gpio_map->dc, 0);
	gp_spi_write(self->spi_fd, cmd);
}

void gp_display_spi_data(struct gp_display_spi *self, uint8_t data)
{
	gp_gpio_write(&self->gpio_map->dc, 1);
	gp_spi_write(self->spi_fd, data);
}

void gp_display_spi_cmd_data(struct gp_display_spi *self, uint8_t cmd, uint8_t data)
{
	gp_display_spi_cmd(self, cmd);
	gp_display_spi_data(self, data);
}

int gp_display_spi_busy_edge_set(struct gp_display_spi *self, enum gp_gpio_edge edge)
{
	return gp_gpio_edge_set(&self->gpio_map->busy, edge);
}

void gp_display_spi_data_write(struct gp_display_spi *self,
                               const uint8_t *data, size_t data_size)
{
	gp_gpio_write(&self->gpio_map->dc, 1);
	gp_spi_send(self->spi_fd, data, data_size);
}

void gp_display_spi_wait_ready(struct gp_display_spi *self, int ready)
{
	int timeout = 1000;

	GP_DEBUG(5, "Waiting for busy signal");

	usleep(10000);

	while (timeout-- > 0) {
		if (gp_gpio_read(&self->gpio_map->busy) == ready) {
			GP_DEBUG(5, "Display ready");
			return;
		}

		usleep(5000);
	}

	GP_WARN("Timeouted while waiting for busy signal");
}
