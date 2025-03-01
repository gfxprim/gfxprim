// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2023-2025 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_linux_gpio.h
 * @brief A Linux userspace GPIO.
 */

#ifndef GP_LINUX_GPIO_H
#define GP_LINUX_GPIO_H

#include <stdint.h>

/** @brief A GPIO direction to be requested. */
enum gp_gpio_dir {
	/** @brief GPIO is input. */
	GP_GPIO_IN = 0,
	/** @brief GPIO is output. */
	GP_GPIO_OUT = 1,
};

/** @brief An edge direction for IRQ. */
enum gp_gpio_edge {
	/**
	 * @brief Disables edge events.
	 *
	 * - Disables GPIO edge events when passed to gp_gpio_edge_set().
	 * - Is returned as an error from gp_gpio_edge_read().
	 */
	GP_GPIO_EDGE_NONE = 0x00,
	/**
	 * @brief Enables events for falling edge.
	 */
	GP_GPIO_EDGE_FALL = 0x01,
	/**
	 * @brief Enables events for rising edge.
	 */
	GP_GPIO_EDGE_RISE = 0x10,
	/**
	 * @brief Enables events for both falling and rising edges.
	 */
	GP_GPIO_EDGE_BOTH = GP_GPIO_EDGE_FALL | GP_GPIO_EDGE_RISE,
};

/**
 * @brief A single GPIO description.
 */
struct gp_gpio {
	/**
	 * @brief GPIO number.
	 *
	 * In kernel this is called offset.
	 */
	const uint16_t nr;
	/**
	 * @brief A GPIO direction.
	 *
	 * Either input or output as described in enum gp_gpio_dir.
	 */
	const uint16_t dir:1;
	/**
	 * @brief Index into gpio request.
	 *
	 * This is internal, do not touch.
	 */
	uint16_t idx;
	/**
	 * @brief A file descriptor for the GPIO.
	 *
	 * In the case that GPIO is configured as edge source this can be
	 * passed to poll() and events must be read by gp_gpio_edge_read().
	 */
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
 * @param consumer This describes who uses the GPIOs and is shown in gpioinfo.
 *
 * @return Non-zero on success (all GPIOs were exported succesfully), non-zero
 *         otherwise.
 */
int gp_gpio_export(struct gp_gpio *gpio, unsigned int gpio_cnt,
                   const char *consumer);

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

/**
 * @brief Sets or resets a GPIO edge trigger.
 *
 * If set as edge interrupt source the GPIO file descriptor can be passed to
 * poll(2) with POLLIN for asynchronous edge notification and events are read
 * with gp_gpio_edge_read().
 *
 * @param self An input GPIO.
 * @param edge Edge to watch for.
 * @return A zero on success, non-zero otherwise.
 */
int gp_gpio_edge_set(struct gp_gpio *self, enum gp_gpio_edge edge);

/**
 * @brief Reads event from the GPIO.
 *
 * Calling this function when there are no events queued on the file descriptor
 * will block until an event arrives.
 *
 * @param self An input GPIO.
 * @return A which edge has been read either GP_GPIO_EDGE_FALL or
 *         GP_GPIO_EDGE_RISE. On a failure GP_GPIO_EDGE_NONE is returned.
 */
enum gp_gpio_edge gp_gpio_edge_read(struct gp_gpio *self);

#endif /* GP_LINUX_GPIO_H */
