// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2023-2025 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_linux_spi.h
 * @brief A Linux SPI.
 */

#ifndef GP_LINUX_SPI_H
#define GP_LINUX_SPI_H

#include <linux/spi/spidev.h>

/**
 * @brief Opens an SPI bus.
 *
 * @spi_dev An SPI device dev path, e.g. /dev/spidev0.0
 * @mode An SPI mode, SPI_* constants in the linux/spi/spi.h
 * @speed An SPI speed in Hz.
 *
 * @return A file descriptor or -1 in a case of a failure.
 */
int gp_spi_open(const char *spi_dev, uint8_t mode, uint32_t speed);

/**
 * @brief Writes a single byte to the SPI bus.
 *
 * @spi_fd An SPI bus file descriptor.
 * @byte A byte to transfer.
 *
 * @return Zero on success, non-zero otherwise.
 */
int gp_spi_write(int spi_fd, uint8_t byte);

/**
 * @brief Writes a buffer into SPI.
 *
 * @spi_fd An SPI bus file descriptor.
 * @buf A buffer to be written.
 * @buf_size A buffer size be written.
 *
 * @return Zero on success, non-zero otherwise.
 */
int gp_spi_send(int spi_fd, const uint8_t *buf, size_t buf_size);

/**
 * @brief Closes SPI bus.
 */
void gp_spi_close(int spi_fd);

#endif /* GP_LINUX_SPI_H */
