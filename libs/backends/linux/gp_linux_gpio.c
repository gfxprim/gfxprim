// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2023 Cyril Hrubis <metan@ucw.cz>
 */

#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <inttypes.h>
#include <sys/ioctl.h>

#include <core/gp_debug.h>

#include "gp_linux_gpio.h"

#define GPIO_EXPORT_PATH "/sys/class/gpio/export"
#define GPIO_UNEXPORT_PATH "/sys/class/gpio/unexport"
#define GPIO_DIRECTION_PATH "/sys/class/gpio/gpio%"PRIu16"/direction"
#define GPIO_VALUE_PATH "/sys/class/gpio/gpio%"PRIu16"/value"
#define GPIO_EDGE_PATH "/sys/class/gpio/gpio%"PRIu16"/edge"

static void gpio_unexport(struct gp_gpio *gpio, unsigned int gpio_cnt, int warn_on_fail)
{
	unsigned int i;
	char buf[32];
	int fd;

	GP_DEBUG(4, "Unexporting GPIOs");

	fd = open(GPIO_UNEXPORT_PATH, O_WRONLY);
	if (!fd) {
		GP_FATAL("Failed to open "GPIO_UNEXPORT_PATH": %s",
			 strerror(errno));
		return;
	}

	for (i = 0; i < gpio_cnt; i++) {
		int len;

		len = snprintf(buf, sizeof(buf), "%"PRIu16, gpio[i].nr);

		if (write(fd, buf, len) < 1 && warn_on_fail) {
			GP_WARN("Failed to unexport GPIO %"PRIu16": %s",
				gpio[i].nr, strerror(errno));
		} else {
			GP_DEBUG(4, "GPIO %"PRIu16" unexported", gpio[i].nr);
		}

		if (gpio[i].fd > 0) {
			close(gpio[i].fd);
			gpio[i].fd = -1;
		}
	}

	close(fd);
}

void gp_gpio_unexport(struct gp_gpio *gpio, unsigned int gpio_cnt)
{
	gpio_unexport(gpio, gpio_cnt, 1);
}

int gp_gpio_export(struct gp_gpio *gpio, unsigned int gpio_cnt)
{
	unsigned int i;
	char buf[256];
	int fd;

	GP_DEBUG(4, "Exporting GPIOs");

	fd = open(GPIO_EXPORT_PATH, O_WRONLY);
	if (!fd) {
		GP_FATAL("Failed to open "GPIO_EXPORT_PATH": %s",
			 strerror(errno));
		return 1;
	}

	for (i = 0; i < gpio_cnt; i++) {
		int len;

		len = snprintf(buf, sizeof(buf), "%"PRIu16, gpio[i].nr);

		if (write(fd, buf, len) < 1) {
			GP_FATAL("Failed to export GPIO %"PRIu16": %s",
				 gpio[i].nr, strerror(errno));
			close(fd);
			gpio_unexport(gpio, gpio_cnt, 0);
			return 1;
		}

		GP_DEBUG(5, "GPIO %"PRIu16" exported", gpio[i].nr);
	}

	close(fd);

	GP_DEBUG(4, "Setting GPIO direction");

	for (i = 0; i < gpio_cnt; i++) {
		const char *dir = gpio[i].dir == GP_GPIO_OUT ? "out": "in";
		int len = strlen(dir);

		snprintf(buf, sizeof(buf), GPIO_DIRECTION_PATH, gpio[i].nr);

		/* Wait for the udev to set right group and mode on the newly created directories/files */
		int retries = 100;

		for (;;) {
			fd = open(buf, O_WRONLY);

			if (fd > 0)
				break;

			if (retries-- <= 0)
				break;

			usleep(100000);
		}

		if (fd < 0) {
			GP_FATAL("Faled to open %s: %s", buf, strerror(errno));
			gpio_unexport(gpio, gpio_cnt, 1);
			return 1;
		}

		if (write(fd, dir, len) < 0) {
			GP_FATAL("Failed to write %s: %s", buf, strerror(errno));
			gpio_unexport(gpio, gpio_cnt, 1);
			return 1;
		}

		if (close(fd)) {
			GP_FATAL("Failed to close %s: %s", buf, strerror(errno));
			gpio_unexport(gpio, gpio_cnt, 1);
			return 1;
		}

		GP_DEBUG(5, "GPIO %"PRIu16" set to be %s", gpio[i].nr, dir);
	}

	GP_DEBUG(4, "Opening GPIO values");

	for (i = 0; i < gpio_cnt; i++) {
		int open_flags = gpio[i].dir == GP_GPIO_OUT ? O_WRONLY : O_RDONLY;

		snprintf(buf, sizeof(buf), GPIO_VALUE_PATH, gpio[i].nr);

		gpio[i].fd = open(buf, open_flags | O_CLOEXEC);

		if (gpio[i].fd < 0) {
			GP_FATAL("Failed to open %s: %s", buf, strerror(errno));
			gpio_unexport(gpio, gpio_cnt, 1);
			return 1;
		}

		GP_DEBUG(5, "GPIO %s opened", buf);
	}

	return 0;
}

int gp_gpio_write(struct gp_gpio *self, int val)
{
	char buf[2] = {};

	buf[0] = val ? '1' : '0';

	if (write(self->fd, buf, sizeof(buf)) < 1) {
		GP_WARN("Failed to write GPIO %"PRIu16": %s", self->nr, strerror(errno));
		return 1;
	}

	return 0;
}

int gp_gpio_read(struct gp_gpio *self)
{
	char buf[3];

	if (pread(self->fd, buf, sizeof(buf), 0) < 1) {
		GP_WARN("Failed to read GPIO %"PRIu16": %s", self->nr, strerror(errno));
		return -1;
	}

	switch (buf[0]) {
	case '0':
		return 0;
	case '1':
		return 1;
	default:
		GP_WARN("Invalid value GPIO %"PRIu16, self->nr);
		return 0;
	}
}

#define EDGE_NAME(ename) {.name = ename, .name_len = sizeof(ename)-1}

static struct edge_names {
	const char *name;
	unsigned int name_len;
} edge_names[] = {
	EDGE_NAME("none"),
	EDGE_NAME("falling"),
	EDGE_NAME("rising"),
	EDGE_NAME("both"),
};

int gp_gpio_edge_set(struct gp_gpio *self, enum gp_gpio_edge edge)
{
	char buf[256];

	snprintf(buf, sizeof(buf), GPIO_EDGE_PATH, self->nr);

	GP_DEBUG(4, "Setting GPIO %"PRIu16" edge to %s",
	         self->nr, edge_names[edge].name);

	int fd = open(buf, O_WRONLY);
	if (!fd) {
		GP_FATAL("Failed to open %s: %s", buf, strerror(errno));
		return 1;
	}

	ssize_t len = edge_names[edge].name_len;

	if (write(fd, edge_names[edge].name, len) != len) {
		GP_FATAL("Failed to write %s to %s: %s",
		          edge_names[edge].name, buf, strerror(errno));
		return 1;
	}

	if (close(fd)) {
		GP_FATAL("Failed to close %s: %s", buf, strerror(errno));
		return 1;
	}

	gp_gpio_read(self);

	return 0;
}
