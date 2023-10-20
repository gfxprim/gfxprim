// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2023 Cyril Hrubis <metan@ucw.cz>
 */

#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <inttypes.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

#include <core/gp_debug.h>

#include "gp_linux_spi.h"

int gp_spi_open(const char *spi_dev, uint8_t mode, uint32_t speed)
{
	int fd;

	fd = open(spi_dev, O_RDWR);
	if (fd < 0) {
		GP_FATAL("Failed to open '%s': %s", spi_dev, strerror(errno));
		return 1;
	}

	/* 0 == 8bits */
	uint8_t bits = 0;
	if (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits)) {
		GP_FATAL("Failed to set SPI bits per word: %s", strerror(errno));
		goto err0;
	}

	if (ioctl(fd, SPI_IOC_WR_MODE, &mode)) {
		GP_FATAL("Failed to set SPI mode: %s", strerror(errno));
		goto err0;
	}

	if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed)) {
		GP_FATAL("Failed to set SPI speed: %s", strerror(errno));
		goto err0;
	}

	return fd;
err0:
	close(fd);
	return -1;
}

int gp_spi_write(int spi_fd, uint8_t byte)
{
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)&byte,
		.rx_buf = (unsigned long)NULL,
		.len = 1,
	};

	if (ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr) < 1) {
		GP_WARN("Failed to send SPI message: %s", strerror(errno));
		return 1;
	}

	return 0;
}

int gp_spi_transfer(int spi_fd, const uint8_t *tx_buf,
                    uint8_t *rx_buf, size_t len)
{
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx_buf,
		.rx_buf = (unsigned long)rx_buf,
		.len = len,
	};

	if (ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr) < 1) {
		GP_WARN("Failed to send SPI message: %s", strerror(errno));
		return 1;
	}

	return 0;
}

void gp_spi_close(int spi_fd)
{
	if (close(spi_fd))
		GP_FATAL("Failed to close SPI: %s", strerror(errno));
}
