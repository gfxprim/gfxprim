// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2023-2025 Cyril Hrubis <metan@ucw.cz>
 */

#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <inttypes.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <sys/uio.h>

#include <core/gp_common.h>
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

#define MAX_TRANSFER_SIZE 4096u

int gp_spi_send(int spi_fd, const uint8_t *buf, size_t buf_size)
{
	size_t iovcnt = (buf_size + MAX_TRANSFER_SIZE - 1)/MAX_TRANSFER_SIZE;
	struct iovec iov[iovcnt];
	size_t i;

	for (i = 0; i < iovcnt; i++) {
		size_t len = GP_MIN(MAX_TRANSFER_SIZE, buf_size);

		iov[i].iov_base = (void*)buf;
		iov[i].iov_len = len;

		buf += len;
		buf_size -= len;
	}

	if (writev(spi_fd, iov, iovcnt) <= 0) {
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
