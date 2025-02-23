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
	struct gp_gpio_map *gpio_map;

	int spi_fd;

	uint16_t w;
	uint16_t h;
};

int gp_display_spi_init(struct gp_display_spi *self,
                        const char *spi_dev, uint8_t mode, uint32_t speed,
			struct gp_gpio_map *map, uint16_t w, uint16_t h);

void gp_display_spi_exit(struct gp_display_spi *self);

/**
 * @brief Sends a single command byte to the display.
 *
 * Sets the dc pin low and writes a command byte to SPI bus.
 *
 * @self A SPI display.
 * @cmd A byte to be send.
 */
void gp_display_spi_cmd(struct gp_display_spi *self, uint8_t cmd);

/**
 * @brief Sends a single data byte to the display.
 *
 * Sets the dc pin high and writes a data byte to SPI bus.
 *
 * @self A SPI display.
 * @data A byte to be send.
 */
void gp_display_spi_data(struct gp_display_spi *self, uint8_t data);

/**
 * @brief Sends a command followed by a data.
 *
 * This is shortcut for sending a command followed by a single byte of data.
 * @self A SPI display.
 * @cmd A command byte to be send.
 * @data A data byte to be send.
 */
void gp_display_spi_cmd_data(struct gp_display_spi *self, uint8_t cmd, uint8_t data);

/**
 * @brief Runs a SPI data transfer.
 *
 * Sets the dc pin high and does SPI transfer.
 *
 * @self A SPI display.
 * @tx_buf A transmit buffer, can be NULL.
 * @rx_buf A receive buffer, can be NULL.
 * @len The size of tx and rx buffers.
 */
void gp_display_spi_data_transfer(struct gp_display_spi *self,
                                  const uint8_t *tx_buf, uint8_t *rx_buf, size_t len);

/**
 * @brief Sets up an GPIO as an interrupt source.
 *
 * The GPIO busy file descriptor can be passed to poll() with POLLPRI in order
 * to get asynchronous edge change notification. Note that you need to set up
 * the poll() handler elsewhere, this only enables the kernel functionality.
 *
 * @self A SPI display.
 * @edge An GPIO edge pass GP_GPIO_EDGE_NONE to disable edge notifications.
 *
 * @return Zero on success, non-zero otherwise.
 */
int gp_display_spi_busy_edge_set(struct gp_display_spi *self, enum gp_gpio_edge edge);

/**
 * @brief Spin wait for busy signal.
 *
 * Spins in place until busy signal == ready.
 *
 * @self A SPI display.
 * @ready Value when display is ready, 0 == GND, 1 == VDD.
 */
void gp_display_spi_wait_ready(struct gp_display_spi *self, int ready);

#endif /* GP_DISPLAY_SPI_H */
