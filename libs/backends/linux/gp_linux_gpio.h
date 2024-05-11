// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2023 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_linux_gpio.h
 * @brief A Linux userspace GPIO.
 */

#ifndef GP_LINUX_GPIO_H
#define GP_LINUX_GPIO_H

#include <stdint.h>

enum gp_gpio_dir {
	GP_GPIO_IN = 0,
	GP_GPIO_OUT = 1,
};

struct gp_gpio {
	const uint16_t nr;
	/* enum gp_gpio_dir */
	const uint16_t dir:1;
	int fd;
};

/**
 * @brief Closes and unexports previously exported GPIOs
 *
 * @param gpio An array of GPIO descriptions
 * @param gpio_cnt A number of GPIOs in the gpio array
 */
void gp_gpio_unexport(struct gp_gpio *gpio, unsigned int gpio_cnt);

/**
 * @brief Exports and opens GPIOs
 *
 * Export and opens all GPIOs in the description atomically.
 *
 * @param gpio An array of GPIO descriptions
 * @param gpio_cnt A number of GPIOs in the gpio array
 * @return Non-zero on success (all GPIOs were exported succesfully), non-zero
 *         otherwise.
 */
int gp_gpio_export(struct gp_gpio *gpio, unsigned int gpio_cnt);

/**
 * @brief Writes a GPIO value
 *
 * @param self An output GPIO
 * @param val A value, zero turns the GPIO off, non-zero on.
 */
int gp_gpio_write(struct gp_gpio *self, int val);

/**
 * @brief Reads a GPIO value.
 *
 * @param self An input GPIO
 * @param return A GPIO state.
 */
int gp_gpio_read(struct gp_gpio *self);

/** @brief An edge direction for IRQ */
enum gp_gpio_edge {
	GP_GPIO_EDGE_NONE,
	GP_GPIO_EDGE_FALL,
	GP_GPIO_EDGE_RISE,
	GP_GPIO_EDGE_BOTH,
};

/**
 * @brief Sets or resets a GPIO edge trigger.
 *
 * If set as edge interrupt source the GPIO file descriptor can be passed to
 * poll(2) with POLLPRI for asynchronous edge notification.
 *
 * @param self An input GPIO.
 * @param edge Edge to watch for.
 * @return A zero on success, non-zero otherwise.
 */
int gp_gpio_edge_set(struct gp_gpio *self, enum gp_gpio_edge edge);

#endif /* GP_LINUX_GPIO_H */
