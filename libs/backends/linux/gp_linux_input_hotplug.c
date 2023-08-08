// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2008-2023 Cyril Hrubis <metan@ucw.cz>
 */

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/inotify.h>

#include <core/gp_debug.h>
#include <backends/gp_linux_input.h>
#include <backends/gp_backend_input.h>

#define DEV_PATH "/dev/input/"
#define BUF_LEN 1024

struct linux_input_hotplug {
	gp_backend_input input;
	gp_backend *backend;
	int inotify_fd;
};

static int input_walk(gp_backend *backend)
{
	DIR *dir = opendir(DEV_PATH);
	struct dirent *dir_ent;
	char dev[BUF_LEN];

	if (!dir) {
		GP_WARN("Failed to open '" DEV_PATH "': %s", strerror(errno));
		return 1;
	}

	errno = 0;

	while ((dir_ent = readdir(dir)) != NULL) {
		if (!strncmp(dir_ent->d_name, "event", 5)) {
			snprintf(dev, BUF_LEN, "%s%s", DEV_PATH,
			         dir_ent->d_name);
			if (gp_linux_input_new(dev, backend))
				return 1;
		}
	}

	if (errno)
		return 1;

	return 0;
}

static int input_hotplug_read(gp_fd *fd)
{
	struct linux_input_hotplug *hotplug = fd->priv;
	char buf[BUF_LEN], str[BUF_LEN];
	int len, i;
	struct inotify_event *ev;

	len = read(hotplug->inotify_fd, buf, sizeof(buf));
	if (len < 0)
		return 1;

	for (i = 0; i < len;) {
		ev = (struct inotify_event*) &buf[i];

		if (ev->len && !strncmp(ev->name, "event", 5)) {

			snprintf(str, BUF_LEN, "%s%s", DEV_PATH,
			         ev->name);

			if (ev->mask & IN_CREATE) {
				GP_DEBUG(4, "Plugging new device '%s'.", str);
				gp_linux_input_new(str, hotplug->backend);
			}

			if (ev->mask & IN_DELETE) {
				GP_DEBUG(4, "Unplugging removed device '%s'.", str);
				//dev_unplug(str);
			}
		}

		i += ev->len + sizeof(struct inotify_event);
	}

	return 0;
}

static void input_hotplug_destroy(gp_backend_input *self)
{
	struct linux_input_hotplug *hotplug = GP_CONTAINER_OF(self, struct linux_input_hotplug, input);

	GP_DEBUG(1, "Destroying Linux input hotplug");

	close(hotplug->inotify_fd);
	free(hotplug);
}

int gp_linux_input_hotplug_new(gp_backend *backend)
{
	struct linux_input_hotplug *hotplug = malloc(sizeof(struct linux_input_hotplug));
	int wd;

	if (!hotplug) {
		GP_DEBUG(1, "Malloc failed :(");
		return 1;
	}

	hotplug->inotify_fd = inotify_init();
	if (hotplug->inotify_fd < 0) {
		GP_DEBUG(1, "Failed to intialize inotify: %s", strerror(errno));
		goto err0;
	}

	wd = inotify_add_watch(hotplug->inotify_fd, DEV_PATH, IN_CREATE | IN_DELETE);
	if (wd < 0) {
		GP_DEBUG(1, "Failed to add inotify watch: %s", strerror(errno));
		goto err1;
	}

	GP_DEBUG(1, "Linux input hotplug initialized");

	hotplug->input.destroy = input_hotplug_destroy;
	hotplug->backend = backend;

	gp_dlist_push_head(&backend->input_drivers, &hotplug->input.list_head);

	gp_backend_fds_add(backend, hotplug->inotify_fd, POLLIN, input_hotplug_read, hotplug);

	if (input_walk(backend))
		goto err2;

	return 0;
err2:
	gp_backend_fds_rem(backend, hotplug->inotify_fd);
	gp_dlist_rem(&backend->input_drivers, &hotplug->input.list_head);
err1:
	close(hotplug->inotify_fd);
err0:
	free(hotplug);
	return 1;
}
