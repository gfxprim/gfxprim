// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2023 Cyril Hrubis <metan@ucw.cz>
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <linux/input.h>

#include <core/gp_debug.h>
#include <core/gp_common.h>

#include <backends/gp_linux_input.h>

struct linux_input {
	gp_backend_input input;
	gp_backend *backend;

	/* fd */
	int fd;

	/* to store rel coordinates */
	int rel_x;
	int rel_y;
	int rel_wheel;

	uint8_t rel_flag;

	/* to store abs coordinates */
	int abs_x;
	int abs_y;
	int abs_press;

	int abs_min_x;
	int abs_max_x;
	int abs_max_y;
	int abs_min_y;
	int abs_press_max;

	uint8_t abs_flag_x:1;
	uint8_t abs_flag_y:1;
	uint8_t abs_pen_flag:1;

	uint8_t abs_swap:1;
	uint8_t abs_mirror_x:1;
	uint8_t abs_mirror_y:1;
};

static void input_rel(struct linux_input *self, struct input_event *ev)
{
	GP_DEBUG(4, "Relative event");

	switch (ev->code) {
	case REL_X:
		self->rel_x = ev->value;
		self->rel_flag |= 1;
	break;
	case REL_Y:
		self->rel_y = ev->value;
		self->rel_flag |= 1;
	break;
	case REL_WHEEL:
		self->rel_wheel = ev->value;
		self->rel_flag |= 2;
	break;
	default:
		GP_DEBUG(3, "Unhandled code %i", ev->code);
	}
}

static void input_abs(struct linux_input *self, struct input_event *ev)
{
	GP_DEBUG(4, "Absolute event");

	switch (ev->code) {
	case ABS_X:
		self->abs_x = ev->value;
		self->abs_flag_x = 1;
		GP_DEBUG(4, "ABS X %i", ev->value);
	break;
	case ABS_Y:
		self->abs_y = ev->value;
		self->abs_flag_y = 1;
		GP_DEBUG(4, "ABS Y %i", ev->value);
	break;
	case ABS_PRESSURE:
		self->abs_press = ev->value;
	break;
	default:
		GP_DEBUG(3, "Unhandled code %i", ev->code);
	}
}

static void input_key(struct linux_input *self,
                      gp_ev_queue *event_queue,
                      struct input_event *ev)
{
	GP_DEBUG(4, "Key event");

	/*
	 * We need to postpone btn touch down until
	 * we read new coordinates for cursor.
         */
	if (ev->code == BTN_TOUCH) {
		if (ev->value == 0)
			self->abs_pen_flag = 1;
		else
			return;
	}

	gp_ev_queue_push_key(event_queue, ev->code, ev->value, 0);
}

static void do_sync(struct linux_input *self, gp_ev_queue *ev_queue)
{
	if (self->rel_flag) {
		if (self->rel_flag & 1)
			gp_ev_queue_push_rel(ev_queue, self->rel_x, self->rel_y, 0);


		if (self->rel_flag & 2)
			gp_ev_queue_push_wheel(ev_queue, self->rel_wheel, 0);

		self->rel_x = 0;
		self->rel_y = 0;
		self->rel_flag = 0;
	}

	if (self->abs_flag_x || self->abs_flag_y) {
		uint32_t x = 0, y = 0, x_max = 0, y_max = 0;

		if (self->abs_flag_x) {
			/* clipping */
			if (self->abs_x > self->abs_max_x)
				self->abs_x = self->abs_max_x;

			if (self->abs_x < self->abs_min_x)
				self->abs_x = self->abs_min_x;

			x     = self->abs_x - self->abs_min_x;
			x_max = self->abs_max_x - self->abs_min_x;

			self->abs_flag_x = 0;
		}

		if (self->abs_flag_y) {
			/* clipping */
			if (self->abs_y > self->abs_max_y)
				self->abs_y = self->abs_max_y;

			if (self->abs_y < self->abs_min_y)
				self->abs_y = self->abs_min_y;

			y     = self->abs_y - self->abs_min_y;
			y_max = self->abs_max_y - self->abs_min_y;

			self->abs_flag_y = 0;
		}

		if (self->abs_swap) {
			GP_SWAP(x, y);
			GP_SWAP(x_max, y_max);
		}

		if (self->abs_mirror_x)
			x = x_max - x;

		if (self->abs_mirror_y)
			y = y_max - y;

		gp_ev_queue_push_abs(ev_queue, x, y, self->abs_press,
		                     x_max, y_max, self->abs_press_max, 0);

		self->abs_press = 0;

		if (self->abs_pen_flag) {
			gp_ev_queue_push_key(ev_queue, BTN_TOUCH, 1, 0);
			self->abs_pen_flag = 0;
		}
	}
}

static void input_syn(struct linux_input *self,
                      gp_ev_queue *event_queue,
                      struct input_event *ev)
{
	GP_DEBUG(4, "Sync event");

	switch (ev->code) {
	case 0:
		do_sync(self, event_queue);
	break;
	default:
		GP_DEBUG(3, "Unhandled code %i", ev->code);
	}
}

static int input_read(gp_fd *self)
{
	struct linux_input *input = self->priv;
	struct input_event ev;
	int ret;

	ret = read(input->fd, &ev, sizeof(ev));

	if (ret == -1 && errno == EAGAIN)
		return 0;

	if (ret < 1)
		return 1;

	switch (ev.type) {
	case EV_REL:
		input_rel(input, &ev);
	break;
	case EV_ABS:
		input_abs(input, &ev);
	break;
	case EV_KEY:
		input_key(input, input->backend->event_queue, &ev);
	break;
	case EV_SYN:
		input_syn(input, input->backend->event_queue, &ev);
	break;
	default:
		GP_DEBUG(3, "Unhandled type %i", ev.type);
	}

	return 0;
}

static int get_version(int fd)
{
	int ver;

	if (ioctl(fd, EVIOCGVERSION, &ver))
		return -1;

	GP_DEBUG(2, "Input version %u.%u.%u",
	            ver>>16, (ver>>8)&0xff, ver&0xff);

	return 0;
}

/*
 * Returns size on success just as read()
 */
static int get_name(int fd, char *buf, size_t buf_len)
{
	int ret;

	if ((ret = ioctl(fd, EVIOCGNAME(buf_len), buf)) <= 0)
		return -1;

	return ret;
}

static void print_name(int fd)
{
	char name[64];

	if (gp_get_debug_level() < 2)
		return;

	if (get_name(fd, name, sizeof(name)) > 0)
		GP_DEBUG(2, "Input device name '%s'", name);
}

static void try_load_callibration(struct linux_input *self)
{
	long bit = 0;
	int abs[6];

	ioctl(self->fd, EVIOCGBIT(EV_ABS, EV_ABS), &bit);

	if (!bit) {
		GP_DEBUG(3, "Not an absolute input device");
		return;
	}

	if (!ioctl(self->fd, EVIOCGABS(ABS_X), abs)) {
		GP_DEBUG(3, "ABS X = <%i,%i> Fuzz %i Flat %i",
                            abs[1], abs[2], abs[3], abs[4]);
		self->abs_min_x = abs[1];
		self->abs_max_x = abs[2];
	}

	if (!ioctl(self->fd, EVIOCGABS(ABS_Y), abs)) {
		GP_DEBUG(3, "ABS Y = <%i,%i> Fuzz %i Flat %i",
                            abs[1], abs[2], abs[3], abs[4]);
		self->abs_min_y = abs[1];
		self->abs_max_y = abs[2];
	}

	if (!ioctl(self->fd, EVIOCGABS(ABS_PRESSURE), abs)) {
		GP_DEBUG(3, "ABS P = <%i,%i> Fuzz %i Flat %i",
                            abs[1], abs[2], abs[3], abs[4]);
		self->abs_press_max = abs[2];
	}
}
/*
static int input_by_name(const char *dev_name)
{
	DIR *input = opendir("/dev/input");
	struct dirent *ent;
	char name[128];

	if (!input) {
		GP_WARN("Failed to open '/dev/input': %s", strerror(errno));
		return -1;
	}

	while ((ent = readdir(input))) {
		if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, ".."))
			continue;

		int fd = open(ent->d_name, O_RDONLY | O_NONBLOCK);

		if (fd < 0) {
			GP_WARN("Failed to open '%s': %s", ent->d_name, strerror(errno));
			continue;
		}

		int ret = get_name(fd, name, sizeof(name));

		if (ret < 0 || ret >= (int)sizeof(name)) {
			GP_WARN("Failed to get '%s' device name", ent->d_name);
			continue;
		}

		if (!strcmp(name, dev_name)) {
			GP_DEBUG(1, "Found device name='%s' dev='%s'", name, ent->d_name);
			return fd;
		}

		close(fd);
	}

	GP_DEBUG(1, "No input device name='%s' found", dev_name);
	return -1;
}
*/
static struct linux_input *new_input_driver(int fd)
{
	struct linux_input *ret;

	if (get_version(fd)) {
		GP_DEBUG(1, "Failed ioctl(), not a input device?");
		return NULL;
	}

	print_name(fd);

	ret = malloc(sizeof(struct linux_input));
	if (!ret) {
		GP_DEBUG(1, "Malloc failed :(");
		return NULL;
	}

	ret->fd = fd;

	ret->rel_x = 0;
	ret->rel_y = 0;
	ret->rel_flag = 0;

	ret->abs_x = 0;
	ret->abs_y = 0;
	ret->abs_press = 0;
	ret->abs_flag_x = 0;
	ret->abs_flag_y = 0;
	ret->abs_pen_flag = 1;

	ret->abs_swap = 0;
	ret->abs_mirror_x = 0;
	ret->abs_mirror_y = 0;

	try_load_callibration(ret);

	return ret;
}

/*
struct devstr_params {
	char *dev_name;
	char *dev_path;

	int abs_swap;
	int abs_mirror_x;
	int abs_mirror_y;

	int abs_min_x;
	int abs_max_x;
	int abs_min_y;
	int abs_max_y;
};

struct linux_input *struct linux_input_by_devstr(const char *devstr)
{
	struct linux_input *ret = NULL;
	char *str = strdup(devstr);
	char *tok, *s, *save;

	if (!str) {
		GP_WARN("Malloc failed :-(");
		return NULL;
	}

	s = str;

	while ((tok = strtok_r(s, ":", &save))) {
		char *tsave, *tid = strtok_r(tok, "=", &tsave);

		if (!strcmp(tid, "name")) {
			if (!ret)
				ret = struct linux_input_by_name(tsave);
		} if (!strcmp(tid, "path")) {
			if (!ret)
				ret = struct linux_input_open(tsave);
		} else if (!strcmp(tid, "abs_swap")) {
			if (!ret)
				goto nodev;
			ret->abs_swap = 1;
		} else if (!strcmp(tid, "abs_mirror_x")) {
			if (!ret)
				goto nodev;
			ret->abs_mirror_x = 1;
		} else if (!strcmp(tid, "abs_mirror_y")) {
			if (!ret)
				goto nodev;
			ret->abs_mirror_y = 1;
		} else if (!strcmp(tid, "abs_min_x")) {
			if (!ret)
				goto nodev;
			ret->abs_min_x = atoi(tsave);
		} else if (!strcmp(tid, "abs_max_x")) {
			if (!ret)
				goto nodev;
			ret->abs_max_x = atoi(tsave);
		} else if (!strcmp(tid, "abs_min_y")) {
			if (!ret)
				goto nodev;
			ret->abs_min_y = atoi(tsave);
		} else if (!strcmp(tid, "abs_max_y")) {
			if (!ret)
				goto nodev;
			ret->abs_max_y = atoi(tsave);
		} else {
			GP_WARN("Invalid key '%s'", tok);
			goto exit;
		}

		s = NULL;
	}

exit:
	free(str);
	return ret;
nodev:
	free(str);
	GP_WARN("No device open");
	return NULL;
}

struct linux_input *struct linux_input_by_name(const char *name)
{
	int fd = input_by_name(name);

	if (fd < 0)
		return NULL;

	return new_input_driver(fd);
}
*/

static void input_destroy(gp_backend_input *self)
{
	struct linux_input *input = GP_CONTAINER_OF(self, struct linux_input, input);

	GP_DEBUG(1, "Closing input device");
	print_name(input->fd);

	gp_backend_fds_rem(input->backend, input->fd);
	gp_dlist_rem(&input->backend->input_drivers, &input->input.list_head);

	close(input->fd);
	free(input);
}

int gp_linux_input_new(const char *dev_path, gp_backend *backend)
{
	GP_DEBUG(2, "Opening '%s'", dev_path);

	int fd = open(dev_path, O_RDONLY | O_NONBLOCK);

	if (fd < 0) {
		GP_FATAL("Failed to open '%s': %s", dev_path, strerror(errno));
		return 1;
	}

	struct linux_input *input = new_input_driver(fd);
	if (!input) {
		close(fd);
		return 1;
	}

	input->backend = backend;
	input->input.destroy = input_destroy;

	gp_backend_fds_add(backend, input->fd, POLLIN, input_read, input);
	gp_dlist_push_head(&backend->input_drivers, &input->input.list_head);

	return 0;
}
