// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2025 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_linux_backlight.h
 * @brief A sysfs backlight driver for a Linux.
 */

#ifndef BACKENDS_GP_LINUX_BACKLIGHT_H
#define BACKENDS_GP_LINUX_BACKLIGHT_H

#include <stdint.h>

/**
 * @brief Linux backlight sysfs driver.
 */
struct gp_linux_backlight {
	/** @brief Maximal backlight value read from sysfs. */
	uint32_t brightness_max;
	/** @brief A sysfs path to the brightness file. */
	char brightness_path[];
};

/**
 * @brief Initialize linux backlight driver.
 *
 * Looks up a backlight driver in the /sys/class/backlight/ directory.
 *
 * @return An initialized backlight driver or NULL if not found.
 */
struct gp_linux_backlight *gp_linux_backlight_init(void);

/**
 * @brief Destroy Linux backlight driver.
 *
 * @param self A Linux backlight driver.
 */
void gp_linux_backlight_exit(struct gp_linux_backlight *self);

/**
 * @brief Increase backlight intensity.
 *
 * @param self A Linux backlight driver.
 * @return A new brightness in percents.
 */
int gp_linux_backlight_inc(struct gp_linux_backlight *self);

/**
 * @brief Decrease backlight intensity.
 *
 * @param self A Linux backlight driver.\
 * @return A new brightness in percents.
 */
int gp_linux_backlight_dec(struct gp_linux_backlight *self);

/**
 * @brief Returns backlight intensity in percents.
 *
 * @param self A Linux backlight driver.
 * @return A brightness in percents.
 */
int gp_linux_backlight_get(struct gp_linux_backlight *self);

#endif /* BACKENDS_GP_LINUX_BACKLIGHT_H */
