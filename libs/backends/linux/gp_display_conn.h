// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2023 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef GP_DISPLAY_CONN_H
#define GP_DISPLAY_CONN_H

#include "gp_display_spi.h"

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

/**
 * @brief A display connection description.
 */
struct gp_display_conn_desc {
	/** @brief Display connection name. */
	const char *name;
	/** @brief Display connection description. */
	const char *desc;
	/** @brief GPIO map and SPI device description. */
	struct gp_display_conn *conn;
};

/**
 * @brief A NULL terminated array of available connection descriptions.
 */
extern const struct gp_display_conn_desc gp_display_conn_descs[];

/**
 * @brief Returns display connection by a connection name.
 *
 * WARNING: Using wrong connection mapping may toggle wrong GPIOs which may
 *          damage the hardware!!!
 *
 * @param name A display connection name. Pass "help" for list of available
 *             connections.
 */
struct gp_display_conn *gp_display_conn_by_name(const char *name);

#endif /* GP_DISPLAY_CONN_H */
