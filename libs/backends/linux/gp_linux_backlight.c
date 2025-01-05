// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2025 Cyril Hrubis <metan@ucw.cz>
 */

#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#include <core/gp_debug.h>
#include <utils/gp_strconv.h>
#include <backends/gp_linux_backlight.h>

#define BACKLIGHT_SYSFS_DIR "/sys/class/backlight"

static int get_sysfs_uint(const char *path, uint32_t *res)
{
	int fd = open(path, O_RDONLY);
	char buf[64];
	ssize_t size;

	if (!fd) {
		GP_WARN("Failed to open %s: %s", path, strerror(errno));
		return -1;
	}

	size = read(fd, buf, sizeof(buf)-1);
	if (size < 0) {
		GP_WARN("Failed to read %s: %s", path, strerror(errno));
		return -1;
	}

	if (gp_atou32(buf, size, res) <= 0) {
		GP_WARN("Failed to convert sysfs value");
		return -1;
	}

	return 0;
}

struct gp_linux_backlight *gp_linux_backlight_init(void)
{
	DIR *d;
	struct dirent *ent;
	const char *name = NULL;
	uint32_t max_brightness;
	struct gp_linux_backlight *ret = NULL;
	char path[512];

	GP_DEBUG(1, "Looking up backlight at " BACKLIGHT_SYSFS_DIR);

	d = opendir(BACKLIGHT_SYSFS_DIR);
	if (!d) {
		GP_DEBUG(1, "Failed to open " BACKLIGHT_SYSFS_DIR ": %s", strerror(errno));
		return NULL;
	}

	while ((ent = readdir(d))) {
		if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, ".."))
		    continue;

		name = ent->d_name;
		break;
	}

	if (!name) {
		GP_DEBUG(1, "The " BACKLIGHT_SYSFS_DIR " is empty.");
		goto err0;
	}

	snprintf(path, sizeof(path), BACKLIGHT_SYSFS_DIR "/%s/max_brightness", name);

	if (get_sysfs_uint(path, &max_brightness))
		goto err0;

	GP_DEBUG(1, "Have %s %u", name, (unsigned int)max_brightness);

	snprintf(path, sizeof(path), BACKLIGHT_SYSFS_DIR "/%s/brightness", name);

	ret = malloc(sizeof(struct gp_linux_backlight) + strlen(path) + 1);
	if (!ret) {
		GP_WARN("Malloc failed :-(");
	} else {
		ret->brightness_max = max_brightness;
		strcpy(ret->brightness_path, path);
	}
err0:
	closedir(d);
	return ret;
}

void gp_linux_backlight_exit(struct gp_linux_backlight *self)
{
	GP_DEBUG(1, "Freeing Linux backlight driver '%s'", self->brightness_path);
	free(self);
}

#define TO_PERCENTS(val, max) ((100 * (val) + (max)/2)/(max))

static int inc_sysfs_uint(const char *path, uint32_t max, int32_t inc)
{
	int fd = open(path, O_RDWR);
	char buf[64];
	uint32_t val;
	ssize_t size;

	if (!fd) {
		GP_WARN("Failed to open %s: %s", path, strerror(errno));
		return -1;
	}

	size = read(fd, buf, sizeof(buf));
	if (size < 0) {
		GP_WARN("Failed to read %s: %s", path, strerror(errno));
		return -1;
	}

	if (gp_atou32(buf, size, &val) <= 0) {
		GP_WARN("Failed to convert sysfs value");
		return -1;
	}

	if (inc < 0 && val < (uint32_t)-inc) {
		if (val == 0)
			goto ret;
		val = 0;
		goto write;
	}

	if (val + inc > max) {
		if (val == max)
			goto ret;
		val = max;
		goto write;
	}

	val += inc;

write:
	size = gp_u32toa(buf, val, false);
	size = write(fd, buf, size);
	if (size < 0) {
		GP_WARN("Failed to read %s: %s", path, strerror(errno));
		return -1;
	}

ret:
	return TO_PERCENTS(val, max);
}

int gp_linux_backlight_inc(struct gp_linux_backlight *self)
{
	if (!self)
		return -1;

	int32_t step = GP_MAX(1u, self->brightness_max/20);

	return inc_sysfs_uint(self->brightness_path, self->brightness_max, step);
}

int gp_linux_backlight_dec(struct gp_linux_backlight *self)
{
	if (!self)
		return -1;

	int32_t step = GP_MAX(1u, self->brightness_max/20);

	return inc_sysfs_uint(self->brightness_path, self->brightness_max, -step);
}

int gp_linux_backlight_get(struct gp_linux_backlight *self)
{
	uint32_t brightness;

	if (!self)
		return -1;

	if (get_sysfs_uint(self->brightness_path, &brightness))
		return -1;

	return TO_PERCENTS(brightness, self->brightness_max);
}
