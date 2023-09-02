// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2023 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef GP_LINUX_SPI_H
#define GP_LINUX_SPI_H

#include <linux/spi/spidev.h>

int gp_spi_open(const char *spi_dev, uint8_t mode, uint32_t speed);

int gp_spi_write(int spi_fd, uint8_t byte);

void gp_spi_close(int spi_fd);

#endif /* GP_LINUX_SPI_H */
