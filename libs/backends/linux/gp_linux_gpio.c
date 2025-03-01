// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2023-2025 Cyril Hrubis <metan@ucw.cz>
 */

#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <inttypes.h>
#include <sys/ioctl.h>
#include <linux/gpio.h>

#include <core/gp_debug.h>

#include "gp_linux_gpio.h"

#define GPIO_DEV "/dev/gpiochip0"

void gp_gpio_unexport(struct gp_gpio *gpio, unsigned int gpio_cnt)
{
	GP_DEBUG(4, "Unexporting GPIOs");

	(void) gpio_cnt;

	if (close(gpio->fd))
		GP_WARN("Failed to unexport GPIOs: %s", strerror(errno));
}

int gp_gpio_export(struct gp_gpio *gpio, unsigned int gpio_cnt,
                   const char *consumer)
{
	unsigned int i;
	int fd;

	GP_DEBUG(4, "Exporting GPIOs");

	fd = open(GPIO_DEV, O_RDONLY | O_CLOEXEC);
	if (fd < 0) {
		GP_FATAL("Failed to open "GPIO_DEV": %s", strerror(errno));
		return 1;
	}

	struct gpio_v2_line_request req = {
		.num_lines = gpio_cnt,
		.config = {
			/* Default to output */
			.flags = GPIO_V2_LINE_FLAG_OUTPUT,
			.num_attrs = 1,
		},
	};

	strncpy(req.consumer, consumer, sizeof(req.consumer) - 1);

	req.config.attrs[0].mask = 0;
	req.config.attrs[0].attr.id = GPIO_V2_LINE_ATTR_ID_FLAGS;
	req.config.attrs[0].attr.flags = GPIO_V2_LINE_FLAG_INPUT;

	for (i = 0; i < gpio_cnt; i++) {
		req.offsets[i] = gpio[i].nr;

		if (gpio[i].dir == GP_GPIO_IN)
			req.config.attrs[0].mask |= 1ULL << i;
	}

	GP_DEBUG(0, "Requesting GPIOs on '%s'", GPIO_DEV);

	if (ioctl(fd, GPIO_V2_GET_LINE_IOCTL, &req)) {
		GP_FATAL("Failed to request GPIOs: %s", strerror(errno));
		return 1;
	}

	if (close(fd))
		GP_WARN("Failed to close '%s': %s", GPIO_DEV, strerror(errno));

	for (i = 0; i < gpio_cnt; i++) {
		gpio[i].fd = req.fd;
		gpio[i].idx = i;
	}

	return 0;
}

int gp_gpio_write(struct gp_gpio *self, int val)
{
	uint64_t bit = !!val;

	struct gpio_v2_line_values values = {
		.mask = 1ULL<<self->idx,
		.bits = bit<<self->idx,
	};

	if (ioctl(self->fd, GPIO_V2_LINE_SET_VALUES_IOCTL, &values) == -1) {
		GP_FATAL("Failed to set GPIO %u to %i: %s",
		         self->nr, !!val, strerror(errno));
		return 1;
	}

	return 0;
}

int gp_gpio_read(struct gp_gpio *self)
{
	struct gpio_v2_line_values values = {
		.mask = 1ULL<<self->idx,
	};

	if (ioctl(self->fd, GPIO_V2_LINE_GET_VALUES_IOCTL, &values) == -1) {
		GP_FATAL("Failed to get GPIO %u: %s",
		         self->nr, strerror(errno));
		return -1;
	}

	return !!values.bits;
}

static const char *edge_name(enum gp_gpio_edge edge)
{
	switch (edge) {
	case GP_GPIO_EDGE_NONE:
		return "none";
	case GP_GPIO_EDGE_FALL:
		return "fall";
	case GP_GPIO_EDGE_RISE:
		return "rise";
	case GP_GPIO_EDGE_BOTH:
		return "both";
	default:
		return "invalid";
	}
}

int gp_gpio_edge_set(struct gp_gpio *self, enum gp_gpio_edge edge)
{
	int flags = GPIO_V2_LINE_FLAG_INPUT;

	if (edge & GP_GPIO_EDGE_FALL)
		flags |= GPIO_V2_LINE_FLAG_EDGE_FALLING;

	if (edge & GP_GPIO_EDGE_RISE)
		flags |= GPIO_V2_LINE_FLAG_EDGE_RISING;

	struct gpio_v2_line_config config = {
		.num_attrs = 1,
	};

	config.attrs[0].mask = 1ULL<<self->idx;
	config.attrs[0].attr.id = GPIO_V2_LINE_ATTR_ID_FLAGS;
	config.attrs[0].attr.flags = flags;

	GP_DEBUG(4, "Setting GPIO %u edge to %s", self->nr, edge_name(edge));

	if (ioctl(self->fd, GPIO_V2_LINE_SET_CONFIG_IOCTL, &config)) {
		GP_FATAL("Failed to set GPIO %u edge: %s", self->nr, strerror(errno));
		return 1;
	}

	return 0;
}

enum gp_gpio_edge gp_gpio_edge_read(struct gp_gpio *self)
{
	struct gpio_v2_line_event ev;
	ssize_t ret;

	ret = read(self->fd, &ev, sizeof(ev));
	if (ret < 0) {
		GP_WARN("Event read from GPIO %u failed: %s",
			self->nr, strerror(errno));
		return GP_GPIO_EDGE_NONE;
	}

	if (ret == 0)
		return GP_GPIO_EDGE_NONE;

	switch (ev.id) {
	case GPIO_V2_LINE_EVENT_RISING_EDGE:
		return GP_GPIO_EDGE_RISE;
	case GPIO_V2_LINE_EVENT_FALLING_EDGE:
		return GP_GPIO_EDGE_FALL;
	}

	GP_WARN("Invalid GPIO event id = %u?!", ev.id);

	return GP_GPIO_EDGE_NONE;
}
