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
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <poll.h>

#include <linux/fb.h>
#include <linux/kd.h>
#include <linux/vt.h>

#include "core/GP_Debug.h"
#include "input/GP_InputDriverKBD.h"
#include "backends/GP_LinuxFB.h"

struct fb_priv {
	GP_Context context;
	uint32_t bsize;

	int flag;

	/* console fd, nr and saved data */
	int con_fd;
	int con_nr;
	int last_con_nr;
	int saved_kb_mode;
	
	int fb_fd;
	char path[];
};

/*
 * Allocates and switches to newly allocated console.
 */
static int allocate_console(struct fb_priv *fb, int flag)
{
	struct vt_stat vts;
	int fd, nr;
	char buf[255];

	/* allocate and switch to new console */
	GP_DEBUG(1, "Allocating new console");

	fd = open("/dev/tty1", O_WRONLY);

	if (fd < 0) {
		GP_DEBUG(1, "Opening console /dev/tty1 failed: %s",
		            strerror(errno));
		return -1;
	}
	
	if (ioctl(fd, VT_OPENQRY, &nr) < 0) {
		GP_DEBUG(1, "Failed to ioctl VT_OPENQRY /dev/tty1: %s",
		            strerror(errno));
		close(fd);
		return -1;
	}

	GP_DEBUG(1, "Has been granted tty%i", nr);

	close(fd);

	snprintf(buf, sizeof(buf), "/dev/tty%i", nr);
	fd = open(buf, O_RDWR | O_NONBLOCK);

	if (fd < 0) {
		GP_DEBUG(1, "Opening console %s failed: %s",
		            buf, strerror(errno));
		return -1;
	}
	
	if (ioctl(fd, VT_GETSTATE, &vts) == 0)
		fb->last_con_nr = vts.v_active;
	else
		fb->last_con_nr = -1;

	if (ioctl(fd, VT_ACTIVATE, nr) < 0) {
		GP_DEBUG(1, "Failed to ioctl VT_ACTIVATE %s: %s",
		            buf, strerror(errno));
		close(fd);
		return -1;
	}

	GP_DEBUG(1, "Waiting for tty%i to activate", nr);

	if (ioctl(fd, VT_WAITACTIVE, nr) < 0) {
		GP_DEBUG(1, "Failed to ioctl VT_WAITACTIVE %s: %s",
		            buf, strerror(errno));
		close(fd);
		return -1;
	}

	/* turn off blinking cursor */
	if (ioctl(fd, KDSETMODE, KD_GRAPHICS) < 0) {
		GP_DEBUG(1, "Failed to ioctl KDSETMODE %s: %s",
		            buf, strerror(errno));
		close(fd);
		return -1;
	}

	/* set keyboard to raw mode */
	if (flag) {
		struct termios t;
		cfmakeraw(&t);
		
		if (tcsetattr(fd, TCSANOW, &t) < 0) {
			GP_DEBUG(1, "Failed to tcsetattr(): %s",
			         strerror(errno));
			close(fd);
			return -1;
		}
	
		if (ioctl(fd, KDGKBMODE, &fb->saved_kb_mode)) {
			GP_DEBUG(1, "Failed to ioctl KDGKBMODE %s: %s",
                                 buf, strerror(errno));
			close(fd);
			return -1;
		}

		GP_DEBUG(2, "Previous keyboard mode was '%i'",
                         fb->saved_kb_mode);

		if (ioctl(fd, KDSKBMODE, K_MEDIUMRAW) < 0) {
			GP_DEBUG(1, "Failed to ioctl KDSKBMODE %s: %s",
			         buf, strerror(errno));
			close(fd);
			return -1;
		}
	}

	fb->con_nr = nr;
	fb->con_fd = fd;

	return 0;
}

/* Backend API callbacks */

static void fb_poll(GP_Backend *self)
{
	struct fb_priv *fb = GP_BACKEND_PRIV(self); 
	unsigned char buf[16];
	int i, res;

	res = read(fb->con_fd, buf, sizeof(buf));

	for (i = 0; i < res; i++)
		GP_InputDriverKBDEventPut(&self->event_queue, buf[i]);
}

static void fb_wait(GP_Backend *self)
{
	struct fb_priv *fb = GP_BACKEND_PRIV(self); 

	struct pollfd fd = {.fd = fb->con_fd, .events = POLLIN, .revents = 0};

	if (poll(&fd, 1, -1) > 0)
		fb_poll(self);
	else
		GP_WARN("poll(): %s", strerror(errno));
}

static void fb_exit(GP_Backend *self)
{
	struct fb_priv *fb = GP_BACKEND_PRIV(self); 

	/* unmap framebuffer */
	munmap(fb->context.pixels, fb->bsize);
	close(fb->fb_fd);
	
	/* reset keyboard */
	ioctl(fb->con_fd, KDSETMODE, KD_TEXT);
	
	/* restore keyboard mode */
	if (fb->flag) {
		if (ioctl(fb->con_fd, KDSKBMODE, fb->saved_kb_mode) < 0) {
			GP_DEBUG(1, "Failed to ioctl KDSKBMODE (restore KBMODE)"
                                 " /dev/tty%i: %s", fb->con_nr, strerror(errno));
		}
	}
	
	/* switch back console */
	if (fb->last_con_nr != -1)
		ioctl(fb->con_fd, VT_ACTIVATE, fb->last_con_nr);
	
	close(fb->con_fd);
	free(self);
}

GP_Backend *GP_BackendLinuxFBInit(const char *path, int flag)
{
	GP_Backend *backend;
	struct fb_priv *fb;
	struct fb_fix_screeninfo fscri;
	struct fb_var_screeninfo vscri;
	int fd;
	
	backend = malloc(sizeof(GP_Backend) +
	                 sizeof(struct fb_priv) + strlen(path) + 1);

	if (backend == NULL)
		return NULL;

	fb = GP_BACKEND_PRIV(backend);

	if (allocate_console(fb, flag))
		goto err1;

	/* open and mmap framebuffer */
	GP_DEBUG(1, "Opening framebuffer '%s'", path);

	fd = open(path, O_RDWR);
	
	if (fd < 0) {
		GP_DEBUG(1, "Opening framebuffer failed: %s", strerror(errno));
		goto err2;
	}

	if (ioctl(fd, FBIOGET_FSCREENINFO, &fscri) < 0) {
		GP_DEBUG(1, "Failed to ioctl FBIOGET_FSCREENINFO: %s",
		            strerror(errno));
		goto err3;
	}
	
	if (ioctl(fd, FBIOGET_VSCREENINFO, &vscri) < 0) {
		GP_DEBUG(1, "Failed to ioctl FBIOGET_VSCREENINFO: %s",
		            strerror(errno));
		goto err3;
	}

	GP_DEBUG(1, "Have framebufer %ix%i %s %ibpp", vscri.xres, vscri.yres,
                 vscri.grayscale ? "Gray" : "RGB", vscri.bits_per_pixel);

	/*
	 * Framebuffer is grayscale.
	 */
	if (vscri.grayscale) {
		//TODO
		goto err3;
	}

	enum GP_PixelType pixel_type;
	pixel_type = GP_PixelRGBLookup(vscri.red.length,    vscri.red.offset,
	                               vscri.green.length,  vscri.green.offset,
	                               vscri.blue.length,   vscri.blue.offset,
	                               vscri.transp.length, vscri.transp.offset,
	                               vscri.bits_per_pixel);

	if (pixel_type == GP_PIXEL_UNKNOWN) {
		GP_DEBUG(1, "Unknown pixel type\n");
		goto err3;
	}

	fb->context.pixels = mmap(NULL, fscri.smem_len,
	                          PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED,
				  fd, 0);

	if (fb->context.pixels == MAP_FAILED) {
		GP_DEBUG(1, "mmaping framebuffer failed: %s", strerror(errno));
		goto err3;
	}

	fb->fb_fd = fd;
	fb->bsize  = fscri.smem_len;
	strcpy(fb->path, path);
	fb->flag = flag;

	fb->context.w = vscri.xres;
	fb->context.h = vscri.yres;
	
	fb->context.axes_swap = 0;
	fb->context.x_swap    = 0;
	fb->context.y_swap    = 0;

	fb->context.bpp = vscri.bits_per_pixel;
	fb->context.bytes_per_row  = fscri.line_length;
	fb->context.pixel_type = pixel_type;

	/* update API */
	backend->name          = "Linux FB";
	backend->context       = &fb->context;
	backend->Flip          = NULL;
	backend->UpdateRect    = NULL;
	backend->Exit          = fb_exit;
	backend->SetAttributes = NULL;
	backend->ResizeAck     = NULL;
	backend->Poll          = flag ? fb_poll : NULL;
	backend->Wait          = flag ? fb_wait : NULL;
	backend->fd            = fb->con_fd;

	GP_EventQueueInit(&backend->event_queue, vscri.xres, vscri.yres, 0);

	return backend;
err3:
	close(fd);
err2:
	close(fb->con_fd);
	
	/* reset keyboard */
	ioctl(fb->con_fd, KDSETMODE, KD_TEXT);
	
	/* switch back console */
	if (fb->last_con_nr != -1)
		ioctl(fb->con_fd, VT_ACTIVATE, fb->last_con_nr);
err1:
	free(backend);
	return NULL;
}
