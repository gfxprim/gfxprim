/*****************************************************************************
 * This file is part of gfxprim library.                                     *
 *                                                                           *
 * Gfxprim is free software; you can redistribute it and/or                  *
 * modify it under the terms of the GNU Lesser General Public                *
 * License as published by the Free Software Foundation; either              *
 * version 2.1 of the License, or (at your option) any later version.        *
 *                                                                           *
 * Gfxprim is distributed in the hope that it will be useful,                *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Lesser General Public License for more details.                           *
 *                                                                           *
 * You should have received a copy of the GNU Lesser General Public          *
 * License along with gfxprim; if not, write to the Free Software            *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,                        *
 * Boston, MA  02110-1301  USA                                               *
 *                                                                           *
 * Copyright (C) 2009-2011 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <linux/input.h>

#include "core/GP_Debug.h"

#include "GP_Event.h"
#include "GP_InputDriverLinux.h"

int get_version(int fd)
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
int get_name(int fd, char *buf, size_t buf_len)
{
	int ret;

	if ((ret = ioctl(fd, EVIOCGNAME(buf_len), buf)) <= 0)
		return -1;

	return ret;
}
	
void print_name(int fd)
{
	char name[64];

	if (GP_GetDebugLevel() < 2)
		return;

	if (get_name(fd, name, sizeof(name)) > 0)
		GP_DEBUG(2, "Input device name '%s'", name);
}

void try_load_callibration(struct GP_InputDriverLinux *self)
{
	long bit = 0;
	int abs[5];

	ioctl(self->fd, EVIOCGBIT(EV_ABS, EV_ABS), &bit);

	if (!bit) {
		GP_DEBUG(3, "Not an absolute input device");
		return;
	}

	if (!ioctl(self->fd, EVIOCGABS(ABS_X), abs)) {
		GP_DEBUG(3, "ABS X = <%i,%i>", abs[1], abs[2]);
		self->abs_x_max = abs[2];
	}
	
	if (!ioctl(self->fd, EVIOCGABS(ABS_Y), abs)) {
		GP_DEBUG(3, "ABS Y = <%i,%i>", abs[1], abs[2]);
		self->abs_y_max = abs[2];
	}
	
	if (!ioctl(self->fd, EVIOCGABS(ABS_PRESSURE), abs)) {
		GP_DEBUG(3, "ABS P = <%i,%i>", abs[1], abs[2]);
		self->abs_press_max = abs[2];
	}
}

struct GP_InputDriverLinux *GP_InputDriverLinuxOpen(const char *path)
{
	int fd;
	struct GP_InputDriverLinux *ret;

	GP_DEBUG(2, "Opening '%s'", path);

	fd = open(path, O_RDONLY | O_NONBLOCK);

	if (fd < 0) {
		GP_DEBUG(1, "Failed to open '%s': %s", path, strerror(errno));
		return NULL;
	}

	if (get_version(fd)) {
		GP_DEBUG(1, "Failed ioctl(), not a input device?");
		return NULL;
	}

	print_name(fd);

	ret = malloc(sizeof(struct GP_InputDriverLinux));

	if (ret == NULL) {
		GP_DEBUG(1, "Malloc failed :(");
		close(fd);
		return NULL;
	}

	ret->fd = fd;
	
	ret->rel_x = 0;
	ret->rel_y = 0;
	ret->rel_flag = 0;

	ret->abs_x = 0;
	ret->abs_y = 0;
	ret->abs_press = 0;
	ret->abs_flag = 0;

	try_load_callibration(ret);

	return ret;
}

void GP_InputDriverLinuxClose(struct GP_InputDriverLinux *self)
{
	GP_DEBUG(1, "Closing input device");
	print_name(self->fd);

	close(self->fd);
	free(self);
}

static void input_rel(struct GP_InputDriverLinux *self, struct input_event *ev)
{
	GP_DEBUG(4, "Relative event");

	switch (ev->code) {
	case REL_X:
		self->rel_x = ev->value;
		self->rel_flag = 1;
	break;
	case REL_Y:
		self->rel_y = ev->value;
		self->rel_flag = 1;
	break;
	default:
		GP_DEBUG(3, "Unhandled code %i", ev->code);
	}
}

static void input_abs(struct GP_InputDriverLinux *self, struct input_event *ev)
{
	GP_DEBUG(4, "Absolute event");
	
	switch (ev->code) {
	case ABS_X:
		self->abs_x = ev->value;
		self->abs_flag = 1;
	break;
	case ABS_Y:
		self->abs_y = ev->value;
		self->abs_flag = 1;
	break;
	case ABS_PRESSURE:
		self->abs_press = ev->value;
		self->abs_flag = 1;
	break;
	default:
		GP_DEBUG(3, "Unhandled code %i", ev->code);
	}
}

static void input_key(struct GP_InputDriverLinux *self, struct input_event *ev)
{
	GP_DEBUG(4, "Key event");
	
	(void) self;

	GP_EventPushKey(ev->code, ev->value, NULL);
}

static void do_sync(struct GP_InputDriverLinux *self)
{
	if (self->rel_flag) {
		self->rel_flag = 0;
		GP_EventPushRel(self->rel_x, self->rel_y, NULL);
		self->rel_x = 0;
		self->rel_y = 0;
	}

	if (self->abs_flag) {
		self->abs_flag = 0;
		GP_EventPushAbs(self->abs_x, self->abs_y, self->abs_press,
		                self->abs_x_max, self->abs_y_max,
				self->abs_press_max, NULL);
		self->abs_x = 0;
		self->abs_y = 0;
		self->abs_press = 0;
	}
}

static void input_syn(struct GP_InputDriverLinux *self, struct input_event *ev)
{
	GP_DEBUG(4, "Sync event");
	
	switch (ev->code) {
	case 0:
		do_sync(self);
	break;
	default:
		GP_DEBUG(3, "Unhandled code %i", ev->code);
	}
}

int GP_InputDriverLinuxRead(struct GP_InputDriverLinux *self)
{
	struct input_event ev;
	int ret;

	ret = read(self->fd, &ev, sizeof(ev));

	if (ret == -1 && errno == EAGAIN)
		return 0;

	if (ret < 1)
		return -1;

	switch (ev.type) {
	case EV_REL:
		input_rel(self, &ev);
	break;
	case EV_ABS:
		input_abs(self, &ev);
	break;
	case EV_KEY:
		input_key(self, &ev);
	break;
	case EV_SYN:
		input_syn(self, &ev);
	break;
	default:
		GP_DEBUG(3, "Unhandled type %i", ev.type);
	}

	return 1;
}
