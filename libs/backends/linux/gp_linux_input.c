// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2025 Cyril Hrubis <metan@ucw.cz>
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>

#include <core/gp_debug.h>
#include <core/gp_common.h>
#include <core/gp_clamp.h>

#include <utils/gp_app_cfg.h>
#include <utils/gp_json_reader.h>
#include <utils/gp_json_serdes.h>

#include "gp_linux_input_dev.h"

static struct leds_map {
	int bitflag;
	int code;
} led_map[] = {
	{GP_KBD_LED_NUM_LOCK, LED_NUML},
	{GP_KBD_LED_CAPS_LOCK, LED_CAPSL},
	{GP_KBD_LED_SCROLL_LOCK, LED_SCROLLL},
	{GP_KBD_LED_COMPOSE, LED_COMPOSE},
	{GP_KBD_LED_KANA, LED_KANA},
};

static int set_get_leds(gp_ev_feedback *self, gp_ev_feedback_op *op)
{
	struct linux_input *input = GP_CONTAINER_OF(self, struct linux_input, feedback);
	struct input_event ev[GP_ARRAY_SIZE(led_map) + 1] = {};
	int value;

	switch (op->op) {
	case GP_EV_LEDS_ON:
		input->leds |= op->val;
		value = 1;
	break;
	case GP_EV_LEDS_OFF:
		input->leds ^= ~(op->val);
		value = 0;
	break;
	case GP_EV_LEDS_GET:
		op->val = input->leds;
		return 0;
	break;
	default:
		return 1;
	}

	size_t i;
	int cur = 0;

	for (i = 0; i < GP_ARRAY_SIZE(led_map); i++) {
		if (led_map[i].bitflag & op->val) {
			ev[cur].type = EV_LED;
			ev[cur].code = led_map[i].code;
			ev[cur].value = value;
			cur++;
		}
	}

	ev[cur].type = EV_SYN;
	ev[cur].code = SYN_REPORT;

	ssize_t ret = write(input->fd.fd, ev, (cur+1) * sizeof(struct input_event));

	GP_DEBUG(1, "Writing leds! %zi %s", ret, strerror(errno));

	return 0;
}

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
		GP_DEBUG(3, "Unhandled code 0x%02x", ev->code);
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
		GP_DEBUG(3, "Unhandled code 0x%02x", ev->code);
	}
}

static void input_key(struct linux_input *self,
                      gp_ev_queue *event_queue,
                      struct input_event *ev)
{
	GP_DEBUG(4, "Key %s event", gp_ev_key_name(ev->code));

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

	gp_ev_queue_push_key(event_queue, ev->code, ev->value, 0, 0);
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
			gp_ev_queue_push_key(ev_queue, BTN_TOUCH, 1, 0, 0);
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

static enum gp_poll_event_ret input_read(gp_fd *self)
{
	struct linux_input *input = self->priv;
	struct input_event ev;
	int ret;

	ret = read(input->fd.fd, &ev, sizeof(ev));

	if (ret == -1 && errno == EAGAIN)
		return 0;

	if (ret < 1)
		return 1;

	if (input->device_type == LINUX_INPUT_TOUCHPAD) {
		input_touchpad(input, &ev);
		return 0;
	}

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

#define BITS_PER_LONG (sizeof(long) * 8)
#define NBITS(x) ((((x)-1)/BITS_PER_LONG)+1)

static inline int test_bit(unsigned long *bitmask, unsigned int bit)
{
	return (bitmask[bit / BITS_PER_LONG] >> (bit % BITS_PER_LONG)) & 0x01;
}

static void init_device_type(struct linux_input *self, const char *id)
{
	unsigned long types[NBITS(EV_MAX)] = {};
	unsigned long ev_abs[NBITS(ABS_MAX)] = {};
	unsigned long ev_keys[NBITS(KEY_MAX)] = {};
	int fd = self->fd.fd;

	self->device_type = LINUX_INPUT_NONE;

	if (ioctl(fd, EVIOCGBIT(0, EV_MAX), types) < 0) {
		GP_WARN("Failed to get input device capabilities!");
		return;
	}

	if (test_bit(types, EV_REL)) {
		GP_DEBUG(1, "Input device %s is mouse pointer.", id);
		self->device_type = LINUX_INPUT_MOUSE;
		return;
	}

	if (!test_bit(types, EV_KEY)) {
		GP_DEBUG(1, "Input device %s does not have any keys!", id);
		return;
	}

	if (ioctl(fd, EVIOCGBIT(EV_KEY, KEY_MAX), ev_keys) < 0) {
		GP_WARN("Failed to get input device %s key capabilities!", id);
		return;
	}

	if (test_bit(types, EV_ABS)) {
		if (ioctl(fd, EVIOCGBIT(EV_ABS, ABS_MAX), ev_abs) < 0) {
			GP_WARN("Failed to get input device %s absolute capabilities!", id);
			return;
		}

		if (test_bit(ev_abs, ABS_X) && test_bit(ev_abs, ABS_Y)) {
			if (test_bit(ev_keys, BTN_STYLUS) ||
			    test_bit(ev_keys, BTN_TOOL_PEN)) {
				GP_DEBUG(1, "Input device %s is a tablet.", id);
				self->device_type = LINUX_INPUT_TABLET;
				return;
			}

			if (test_bit(ev_keys, BTN_TOOL_FINGER) &&
			    !test_bit(ev_keys, BTN_TOOL_PEN)) {
				GP_DEBUG(1, "Input device %s is a touchpad.", id);
				self->device_type = LINUX_INPUT_TOUCHPAD;
				return;
			}

			if (test_bit(ev_keys, BTN_TRIGGER) ||
                            test_bit(ev_keys, BTN_A) ||
                            test_bit(ev_keys, BTN_1)) {
				GP_DEBUG(1, "Input device %s is a joystick.", id);
				self->device_type = LINUX_INPUT_JOYSTICK;
				return;
			}

			if (test_bit(ev_keys, BTN_MOUSE)) {
				GP_DEBUG(1, "Input device %s is an absolute mouse.", id);
				self->device_type = LINUX_INPUT_MOUSE;
				return;
			}

			if (test_bit(ev_keys, BTN_TOUCH)) {
				GP_DEBUG(1, "Input device %s is a touchscreen.", id);
				self->device_type = LINUX_INPUT_TOUCHSCREEN;
				return;
			}
		}
	}
}

static struct linux_input *new_input_driver(int fd, const char *dev_path)
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

	memset(ret, 0, sizeof(*ret));

	ret->fd = (gp_fd) {
		.fd = fd,
		.event = input_read,
		.events = GP_POLLIN,
		.priv = ret,
	};

	init_device_type(ret, dev_path);

	if (ret->device_type == LINUX_INPUT_TOUCHSCREEN) {
		ret->abs_pen_flag = 1;
		try_load_callibration(ret);
	}

	return ret;
}

static void input_destroy(gp_backend_input *self)
{
	struct linux_input *input = GP_CONTAINER_OF(self, struct linux_input, input);

	GP_DEBUG(1, "Closing input device");
	print_name(input->fd.fd);

	gp_backend_poll_rem(input->backend, &input->fd);
	gp_dlist_rem(&input->backend->input_drivers, &input->input.list_head);

	gp_ev_queue_feedback_unregister(input->backend->event_queue, &input->feedback);

	GP_DEBUG(1, "Ungrabbing device");

	if (ioctl(input->fd.fd, EVIOCGRAB, 0))
		GP_WARN("Failet to ungrab device: %s", strerror(errno));

	close(input->fd.fd);
	free(input);
}

int gp_linux_input_new(const char *dev_path, gp_backend *backend)
{
	GP_DEBUG(2, "Opening '%s'", dev_path);

	int fd = open(dev_path, O_RDWR | O_NONBLOCK);
	if (fd < 0) {
		int err = errno;
		GP_FATAL("Failed to open '%s': %s", dev_path, strerror(errno));
		errno = err;
		return 1;
	}

	struct linux_input *input = new_input_driver(fd, dev_path);
	if (!input) {
		close(fd);
		return 1;
	}

	input->backend = backend;
	input->input.destroy = input_destroy;

	gp_backend_poll_add(backend, &input->fd);
	gp_dlist_push_head(&backend->input_drivers, &input->input.list_head);

	input->feedback.set_get = set_get_leds;

	gp_ev_queue_feedback_register(backend->event_queue, &input->feedback);

	GP_DEBUG(1, "Grabbing device '%s'", dev_path);

	if (ioctl(fd, EVIOCGRAB, 1))
		GP_WARN("Failet to grab device '%s': %s", dev_path, strerror(errno));

	return 0;
}
