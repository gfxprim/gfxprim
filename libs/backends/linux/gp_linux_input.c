// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2024 Cyril Hrubis <metan@ucw.cz>
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
#include <core/gp_clamp.h>

#include <utils/gp_app_cfg.h>
#include <utils/gp_json_reader.h>
#include <utils/gp_json_serdes.h>

#include <backends/gp_linux_input.h>

struct linux_input {
	gp_backend_input input;
	gp_backend *backend;

	gp_fd fd;

	/* to store rel coordinates */
	int rel_x;
	int rel_y;
	int rel_wheel;

	uint8_t rel_flag;

	/* to store abs coordinates */
	int abs_x;
	int abs_y;
	int abs_press;

	/* Coordinate limits after transformation */
	int abs_x_max;
	int abs_y_max;
	int abs_press_min;
	int abs_press_max;

	/*
	 * Affine transformation matrix
	 *
	 * The multipliers are in 16.16 fixed point format
	 */
	int abs_x_off;
	int abs_y_off;
	int abs_x_mul_x;
	int abs_x_mul_y;
	int abs_y_mul_x;
	int abs_y_mul_y;

	uint8_t abs_flag_x:1;
	uint8_t abs_flag_y:1;
	uint8_t abs_pen_flag:1;
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

	if (self->abs_flag_x && self->abs_flag_y) {
		int64_t abs_x = self->abs_x;
		int64_t abs_y = self->abs_y;

		int x = self->abs_x_off;
		int y = self->abs_y_off;

		x+=(abs_x * self->abs_x_mul_x + abs_y * self->abs_x_mul_y)>>16;
		y+=(abs_x * self->abs_y_mul_x + abs_y * self->abs_y_mul_y)>>16;

		x = GP_CLAMP(x, 0, self->abs_x_max);
		y = GP_CLAMP(y, 0, self->abs_y_max);

		gp_ev_queue_push_abs(ev_queue, x, y, self->abs_press,
		                     self->abs_x_max, self->abs_y_max, self->abs_press_max, 0);

		if (self->abs_pen_flag) {
			gp_ev_queue_push_key(ev_queue, BTN_TOUCH, 1, 0);
			self->abs_pen_flag = 0;
		}
	}

	self->abs_press = 0;
	self->abs_flag_x = 0;
	self->abs_flag_y = 0;
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

	ret = read(input->fd.fd, &ev, sizeof(ev));

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

static int replace_in_name(char ch)
{
	switch (ch) {
	default:
		return '_';
	case 0x21 ... 0x7e:
	case 0x00:
		return ch;
	}
}

static const struct gp_json_struct input_desc[] = {
	GP_JSON_SERDES_INT(struct linux_input, abs_x_max, 0, INT_MIN, INT_MAX),
	GP_JSON_SERDES_INT(struct linux_input, abs_x_mul_x, GP_JSON_SERDES_OPTIONAL, INT_MIN, INT_MAX),
	GP_JSON_SERDES_INT(struct linux_input, abs_x_mul_y, GP_JSON_SERDES_OPTIONAL, INT_MIN, INT_MAX),
	GP_JSON_SERDES_INT(struct linux_input, abs_x_off, GP_JSON_SERDES_OPTIONAL, INT_MIN, INT_MAX),
	GP_JSON_SERDES_INT(struct linux_input, abs_y_max, 0, INT_MIN, INT_MAX),
	GP_JSON_SERDES_INT(struct linux_input, abs_y_mul_x, GP_JSON_SERDES_OPTIONAL, INT_MIN, INT_MAX),
	GP_JSON_SERDES_INT(struct linux_input, abs_y_mul_y, GP_JSON_SERDES_OPTIONAL, INT_MIN, INT_MAX),
	GP_JSON_SERDES_INT(struct linux_input, abs_y_off, GP_JSON_SERDES_OPTIONAL, INT_MIN, INT_MAX),
	{}
};

static int load_callibration_file(struct linux_input *self)
{
	char *fname;
	char input_name[256];
	int ret, i;

	ret = get_name(self->fd.fd, input_name, sizeof(input_name)-6);
	if (ret < 0) {
		GP_WARN("Failed to get input device name!");
		return 0;
	}

	for (i = 0; i < ret; i++)
		input_name[i] = replace_in_name(input_name[i]);

	input_name[ret-1] = '.';
	input_name[ret] = 'j';
	input_name[ret+1] = 's';
	input_name[ret+2] = 'o';
	input_name[ret+3] = 'n';
	input_name[ret+4] = '\0';

	fname = gp_app_cfg_path("gfxprim/input", input_name);

	GP_DEBUG(3, "Trying to load callibration at '%s'", fname);

	self->abs_x_mul_x = (1<<16);
	self->abs_y_mul_x = 0;
	self->abs_x_mul_y = 0;
	self->abs_y_mul_y = (1<<16);
	self->abs_x_off = 0;
	self->abs_y_off = 0;

	if (gp_json_load_struct(fname, input_desc, self))
		return 0;

	GP_DEBUG(3, "Callibration loaded! x_off=%i x_max=%i y_off=%i y_max=%i matrix = [%i, %i, %i %i]",
		 self->abs_x_off, self->abs_x_max, self->abs_y_off, self->abs_y_max,
	         self->abs_x_mul_x, self->abs_x_mul_y, self->abs_y_mul_x, self->abs_y_mul_y);

	return 1;
}

static void try_load_callibration(struct linux_input *self)
{
	long bit = 0;
	int abs[6];
	int fd = self->fd.fd;

	ioctl(fd, EVIOCGBIT(EV_ABS, EV_ABS), &bit);

	if (!bit) {
		GP_DEBUG(3, "Not an absolute input device");
		return;
	}

	if (load_callibration_file(self))
		return;

	self->abs_x_mul_y = 0;
	self->abs_y_mul_x = 0;

	if (!ioctl(fd, EVIOCGABS(ABS_X), abs)) {
		GP_DEBUG(3, "ABS X = <%i,%i> Fuzz %i Flat %i",
                            abs[1], abs[2], abs[3], abs[4]);

		int diff = abs[2] - abs[1];

		if (diff < 0) {
			self->abs_x_off = -diff;
			self->abs_x_max = -diff;
			self->abs_x_mul_x = -(1<<16);
		} else {
			self->abs_x_off = 0;
			self->abs_x_max = diff;
			self->abs_x_mul_x = (1<<16);
		}
	}

	if (!ioctl(fd, EVIOCGABS(ABS_Y), abs)) {
		GP_DEBUG(3, "ABS Y = <%i,%i> Fuzz %i Flat %i",
                            abs[1], abs[2], abs[3], abs[4]);

		int diff = abs[2] - abs[1];

		if (diff < 0) {
			self->abs_y_off = -diff;
			self->abs_y_max = -diff;
			self->abs_y_mul_y = -(1<<16);
		} else {
			self->abs_y_off = 0;
			self->abs_y_max = diff;
			self->abs_y_mul_y = (1<<16);
		}
	}

	if (!ioctl(fd, EVIOCGABS(ABS_PRESSURE), abs)) {
		GP_DEBUG(3, "ABS P = <%i,%i> Fuzz %i Flat %i",
                            abs[1], abs[2], abs[3], abs[4]);
		self->abs_press_min = abs[1];
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

	ret->fd = (gp_fd) {
		.fd = fd,
		.event = input_read,
		.events = GP_POLLIN,
		.priv = ret,
	};

	ret->rel_x = 0;
	ret->rel_y = 0;
	ret->rel_flag = 0;

	ret->abs_x = 0;
	ret->abs_y = 0;
	ret->abs_press = 0;
	ret->abs_flag_x = 0;
	ret->abs_flag_y = 0;
	ret->abs_pen_flag = 1;

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
	print_name(input->fd.fd);

	gp_backend_poll_rem(input->backend, &input->fd);
	gp_dlist_rem(&input->backend->input_drivers, &input->input.list_head);

	close(input->fd.fd);
	free(input);
}

int gp_linux_input_new(const char *dev_path, gp_backend *backend)
{
	GP_DEBUG(2, "Opening '%s'", dev_path);

	int fd = open(dev_path, O_RDONLY | O_NONBLOCK);
	if (fd < 0) {
		int err = errno;
		GP_FATAL("Failed to open '%s': %s", dev_path, strerror(errno));
		errno = err;
		return 1;
	}

	struct linux_input *input = new_input_driver(fd);
	if (!input) {
		close(fd);
		return 1;
	}

	input->backend = backend;
	input->input.destroy = input_destroy;

	gp_backend_poll_add(backend, &input->fd);
	gp_dlist_push_head(&backend->input_drivers, &input->input.list_head);

	return 0;
}
