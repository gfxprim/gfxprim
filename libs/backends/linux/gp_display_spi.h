// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2023-2025 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_display_spi.h
 * @brief An SPI display.
 */
#ifndef GP_DISPLAY_SPI_H
#define GP_DISPLAY_SPI_H

#include <stdint.h>
#include "gp_linux_spi.h"
#include "gp_linux_gpio.h"

/**
 * @brief An GPIO map.
 *
 * This implements GPIO signals for 4-wire SPI display with optional busy
 * signal and backlight.
 */
struct gp_gpio_map {
	union {
		struct gp_gpio gpio[4];
		struct {
			/** @brief Reset pin of the display. */
			struct gp_gpio reset;
			/** @brief Data/command for 4-Wire SPI. */
			struct gp_gpio dc;
			/** @brief Backlight power. */
			struct gp_gpio pwr;
			/** @brief Busy signal. */
			struct gp_gpio busy;
		};
	};
};

/**
 * @brief SPI display.
 */
struct gp_display_spi {
	/** @brief GPIO map for pins. */
	struct gp_gpio_map *gpio_map;

	/** @brief Points to /dev/spiX.X */
	int spi_fd;

	/** @brief Display width in pixels. */
	uint16_t w;
	/** @brief Display height in pixels. */
	uint16_t h;

	struct {
		/** @brief Display x offset in pixels. */
		uint16_t x_off;
		/** @brief Display y offset in pixels. */
		uint16_t y_off;
	};
};

/**
 * @brief Initializes an SPI display.
 *
 * Sets up GPIOs, opens SPI device, etc.
 *
 * @brief self An SPI display structure to be initialized.
 * @brief spi_dev An SPI device e.g. "/dev/spidev0.0".
 * @brief mode An SPI mode, see linux/spi/spi.h for details.
 * @brief speed An SPI speed in Hz.
 * @brief gpio_map A GPIO map for the display.
 * @brief w Display width in pixels.
 * @brief h Display height in pixels.
 *
 * @return Zero on success, non-zero otherwise.
 */
int gp_display_spi_init(struct gp_display_spi *self,
                        const char *spi_dev, uint8_t mode, uint32_t speed,
			struct gp_gpio_map *gpio_map, uint16_t w, uint16_t h);

/**
 * @brief Deinitializes an SPI display.
 *
 * Frees up GPIOs, closes SPI device, etc.
 *
 * @param self An SPI display.
 */
void gp_display_spi_exit(struct gp_display_spi *self);

/**
 * @brief Sends a single command byte to the display.
 *
 * Sets the dc pin low and writes a command byte to SPI bus.
 *
 * @self An SPI display.
 * @cmd A byte to be send.
 */
void gp_display_spi_cmd(struct gp_display_spi *self, uint8_t cmd);

/**
 * @brief Sends a single data byte to the display.
 *
 * Sets the dc pin high and writes a data byte to SPI bus.
 *
 * @self An SPI display.
 * @data A byte to be send.
 */
void gp_display_spi_data(struct gp_display_spi *self, uint8_t data);

/**
 * @brief Sends a command followed by a data.
 *
 * This is shortcut for sending a command followed by a single byte of data.
 * @self An SPI display.
 * @cmd A command byte to be send.
 * @data A data byte to be send.
 */
void gp_display_spi_cmd_data(struct gp_display_spi *self, uint8_t cmd, uint8_t data);

/**
 * @brief Writes a buffer with data to the display.
 *
 * If the buffer is larger than maximal SPI transfer size, it's split into
 * several transfers automatically.
 *
 * @self An SPI display.
 * @data A byte to be send.
 * @data_size A size of the data to be send.
 */
void gp_display_spi_data_write(struct gp_display_spi *self,
                               const uint8_t *data, size_t data_size);

/**
 * @brief Sets up an GPIO as an interrupt source.
 *
 * The GPIO busy file descriptor can be passed to poll() with POLLIN in order
 * to get asynchronous edge change notification. Note that you need to set up
 * the poll() handler elsewhere, this only enables the kernel functionality.
 *
 * @self An SPI display.
 * @edge An GPIO edge pass GP_GPIO_EDGE_NONE to disable edge notifications.
 *
 * @return Zero on success, non-zero otherwise.
 */
int gp_display_spi_busy_edge_set(struct gp_display_spi *self, enum gp_gpio_edge edge);

/**
 * @brief Reads edge event from busy GPIO.
 *
 * If busy GPIO was set up by gp_display_spi_busy_edge_set() events can be
 * polled on the file descriptor and have to be read by this function.
 *
 * This function calls gp_gpio_edge_read() on the busy GPIO file descriptor.
 *
 * @self An SPI display.
 * @return An edge or GP_GPIO_EDGE_NONE on error.
 */
static inline enum gp_gpio_edge gp_display_spi_edge_read(struct gp_display_spi *self)
{
	return gp_gpio_edge_read(&self->gpio_map->busy);
}

/**
 * @brief Spin wait for busy signal.
 *
 * Spins in place until busy signal == ready.
 *
 * @self An SPI display.
 * @ready Value when display is ready, 0 == GND, 1 == VDD.
 */
void gp_display_spi_wait_ready(struct gp_display_spi *self, int ready);

#endif /* GP_DISPLAY_SPI_H */
