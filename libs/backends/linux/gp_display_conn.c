// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2023-2026 Cyril Hrubis <metan@ucw.cz>
 */
#include <string.h>
#include <stdio.h>

#include <core/gp_debug.h>

#include "gp_display_conn.h"

/* WARNING: if you mess up direction you may fry the chip! */

/*
 * The chip select GPIO 8 is handled by the kernel SPI driver.
 */
static struct gp_gpio_map gp_gpio_map_rpi1 = {
	.reset  = {.nr = 17, .dir = GP_GPIO_OUT},
	.dc     = {.nr = 25, .dir = GP_GPIO_OUT},
	.pwr    = {.nr = 13, .dir = GP_GPIO_OUT},
	.busy   = {.nr = 24, .dir = GP_GPIO_IN},
};

static struct gp_display_conn gp_display_rpi_conn1 = {
	.gpio_map = &gp_gpio_map_rpi1,
	.spi_dev = "/dev/spidev0.0",
};

/*
 * The chip select GPIO 7 is handled by the kernel SPI driver.
 */
static struct gp_gpio_map gp_gpio_map_rpi2 = {
	.reset  = {.nr = 27, .dir = GP_GPIO_OUT},
	.dc     = {.nr = 23, .dir = GP_GPIO_OUT},
	.pwr    = {.nr = 12, .dir = GP_GPIO_OUT},
	.busy   = {.nr = 22, .dir = GP_GPIO_IN},
};

static struct gp_display_conn gp_display_rpi_conn2 = {
	.gpio_map = &gp_gpio_map_rpi2,
	.spi_dev = "/dev/spidev0.1",
};

const struct gp_display_conn_desc gp_display_conn_descs[] = {
	{"waveshare", "Waveshare e-ink shield", &gp_display_rpi_conn1},
	{"rpi-1", "RPi (rattula) display 1", &gp_display_rpi_conn1},
	{"rpi-2", "RPi (rattula) display 2", &gp_display_rpi_conn2},
	{},
};

struct gp_display_conn *gp_display_conn_by_name(const char *name)
{
	const struct gp_display_conn_desc *desc;

	if (!name)
		return gp_display_conn_descs[0].conn;

	if (!strcasecmp(name, "help")) {
		printf("Available connection mappings:\n\n");

		for (desc = gp_display_conn_descs; desc->name; desc++) {
			printf("%s - %s\n", desc->name, desc->desc);
			printf("\tSPI:             '%s'\n", desc->conn->spi_dev);
			printf("\tReset:           GPIO%i\n", desc->conn->gpio_map->reset.nr);
			printf("\tData/Cmd:        GPIO%i\n", desc->conn->gpio_map->dc.nr);
			printf("\tPower/Backlight: GPIO%i\n", desc->conn->gpio_map->pwr.nr);
			printf("\tBusy:            GPIO%i\n", desc->conn->gpio_map->busy.nr);
			printf("\n");
		}

		return NULL;
	}

	for (desc = gp_display_conn_descs; desc->name; desc++) {
		if (!strcasecmp(name, desc->name))
			return desc->conn;
	}

	GP_FATAL("Failed to find display connection mapping '%s'", name);

	return NULL;
}
