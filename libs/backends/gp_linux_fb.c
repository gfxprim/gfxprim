// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

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

#include <core/gp_debug.h>
#include "core/gp_pixmap.h"
#include <input/gp_input_driver_kbd.h>
#include <backends/gp_linux_fb.h>

struct fb_priv {
	gp_pixmap pixmap;
	uint32_t bsize;
	void *fb_mem;

	int flags;

	/* console fd, nr and saved data */
	int con_fd;
	int con_nr;
	int last_con_nr;
	int saved_kb_mode;
	struct termios ts;
	int restore_termios;

	int fb_fd;
	char path[];
};

/*
 * Restore console and keyboard mode to whatever was there before.
 */
static void exit_kbd(struct fb_priv *fb)
{
	if (ioctl(fb->con_fd, KDSKBMODE, fb->saved_kb_mode) < 0) {
		GP_DEBUG(1, "Failed to ioctl KDSKBMODE (restore KBMODE)"
		         " /dev/tty%i: %s", fb->con_nr,
	         strerror(errno));
	}

	if (fb->restore_termios) {
		if (tcsetattr(fb->con_fd, TCSANOW, &fb->ts) < 0) {
			GP_WARN("Failed to tcsetattr() (restore termios): %s",
			        strerror(errno));
		}
	}
}

/*
 * Save console mode and set the mode to raw.
 */
static int init_kbd(struct fb_priv *fb)
{
	struct termios t;
	int fd = fb->con_fd;

	if (tcgetattr(fd, &fb->ts)) {
		GP_WARN("Failed to tcgetattr(): %s", strerror(errno));
		fb->restore_termios = 0;
	} else {
		fb->restore_termios = 1;
	}

	cfmakeraw(&t);

	if (tcsetattr(fd, TCSANOW, &t) < 0) {
		GP_DEBUG(1, "Failed to tcsetattr(): %s",
		         strerror(errno));
		close(fd);
		return -1;
	}

	if (ioctl(fd, KDGKBMODE, &fb->saved_kb_mode)) {
		GP_DEBUG(1, "Failed to ioctl KDGKBMODE tty%i: %s",
		         fb->con_nr, strerror(errno));
		close(fd);
		return -1;
	}

	GP_DEBUG(2, "Previous keyboard mode was '%i'",
	         fb->saved_kb_mode);

	if (ioctl(fd, KDSKBMODE, K_MEDIUMRAW) < 0) {
		GP_DEBUG(1, "Failed to ioctl KDSKBMODE tty%i: %s",
		        fb->con_nr, strerror(errno));
		close(fd);
		return -1;
	}

	return 0;
}

/*
 * Allocates and switches to newly allocated console.
 */
static int allocate_console(struct fb_priv *fb, int flags)
{
	struct vt_stat vts;
	int fd, nr = -1;
	char buf[255];
	const char *tty = "/dev/tty";

	if (flags & GP_FB_ALLOC_CON) {
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
		tty = buf;
	}

	fd = open(tty, O_RDWR | O_NONBLOCK);

	if (fd < 0) {
		GP_DEBUG(1, "Opening console %s failed: %s",
		         tty, strerror(errno));
		return -1;
	}

	if (ioctl(fd, VT_GETSTATE, &vts) == 0)
		fb->last_con_nr = vts.v_active;
	else
		fb->last_con_nr = -1;

	if (flags & GP_FB_ALLOC_CON) {
		if (ioctl(fd, VT_ACTIVATE, nr) < 0) {
			GP_DEBUG(1, "Failed to ioctl VT_ACTIVATE %s: %s",
			         tty, strerror(errno));
			close(fd);
			return -1;
		}

		GP_DEBUG(1, "Waiting for %s to activate", tty);

		if (ioctl(fd, VT_WAITACTIVE, nr) < 0) {
			GP_DEBUG(1, "Failed to ioctl VT_WAITACTIVE %s: %s",
			         tty, strerror(errno));
			close(fd);
			return -1;
		}
	}

	fb->con_nr = nr;
	fb->con_fd = fd;

	/* turn off blinking cursor */
	if (ioctl(fd, KDSETMODE, KD_GRAPHICS) < 0) {
		GP_DEBUG(1, "Failed to ioctl KDSETMODE %s: %s",
		         tty, strerror(errno));
		close(fd);
		return -1;
	}

	return 0;
}

static void free_console(struct fb_priv *fb)
{
	/* restore blinking cursor */
	if (ioctl(fb->con_fd, KDSETMODE, KD_TEXT))
		GP_WARN("Failed to ioctl KDSETMODE (restore KDMODE)");

	/* switch back console */
	if (fb->last_con_nr != -1)
		ioctl(fb->con_fd, VT_ACTIVATE, fb->last_con_nr);

	close(fb->con_fd);
}

/* Backend API callbacks */

static void fb_poll(gp_backend *self)
{
	struct fb_priv *fb = GP_BACKEND_PRIV(self);
	unsigned char buf[16];
	int i, res;

	res = read(fb->con_fd, buf, sizeof(buf));

	for (i = 0; i < res; i++)
		gp_input_driver_kbd_event_put(&self->event_queue, buf[i]);
}

static void fb_wait(gp_backend *self)
{
	struct fb_priv *fb = GP_BACKEND_PRIV(self);

	struct pollfd fd = {.fd = fb->con_fd, .events = POLLIN, .revents = 0};

	if (poll(&fd, 1, -1) > 0)
		fb_poll(self);
	else
		GP_WARN("poll(): %s", strerror(errno));
}

static void fb_exit(gp_backend *self)
{
	struct fb_priv *fb = GP_BACKEND_PRIV(self);

	if (fb->flags & GP_FB_SHADOW)
		free(fb->pixmap.pixels);

	/* unmap framebuffer */
	munmap(fb->fb_mem, fb->bsize);
	close(fb->fb_fd);

	if (fb->flags & GP_FB_INPUT_KBD)
		exit_kbd(fb);

	free_console(fb);

	free(self);
}

static void fb_flip_shadow(gp_backend *self)
{
	struct fb_priv *fb = GP_BACKEND_PRIV(self);

	GP_DEBUG(2, "Flipping buffer");

	memcpy(fb->fb_mem, fb->pixmap.pixels, fb->bsize);
}

static void fb_update_rect_shadow(gp_backend *self, gp_coord x0, gp_coord y0,
                                  gp_coord x1, gp_coord y1)
{
	struct fb_priv *fb = GP_BACKEND_PRIV(self);

	GP_DEBUG(2, "Flipping buffer");

	size_t size = ((x1 - x0) * fb->pixmap.bpp) / 8;

	for (;y0 <= y1; y0++) {
		void *src = GP_PIXEL_ADDR(&fb->pixmap, x0, y0);
		void *dst = (char*)fb->fb_mem +
                            y0 * fb->pixmap.bytes_per_row +
                            (x0 * fb->pixmap.bpp)/8;
		memcpy(dst, src, size);
	}
}

gp_backend *gp_linux_fb_init(const char *path, int flags)
{
	gp_backend *backend;
	struct fb_priv *fb;
	struct fb_fix_screeninfo fscri;
	struct fb_var_screeninfo vscri;
	int fd;

	backend = malloc(sizeof(gp_backend) +
	                 sizeof(struct fb_priv) + strlen(path) + 1);

	if (backend == NULL)
		return NULL;

	fb = GP_BACKEND_PRIV(backend);

	if (allocate_console(fb, flags))
		goto err0;

	if (flags & GP_FB_INPUT_KBD) {
		if (init_kbd(fb))
			goto err1;
	}

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
		GP_WARN("Grayscale not implemented!");
		goto err3;
	}

	enum gp_pixel_type pixel_type;
	pixel_type = gp_pixel_rgb_lookup(vscri.red.length, vscri.red.offset,
					 vscri.green.length,
					 vscri.green.offset, vscri.blue.length,
					 vscri.blue.offset,
					 vscri.transp.length,
					 vscri.transp.offset,
					 vscri.bits_per_pixel);

	if (pixel_type == GP_PIXEL_UNKNOWN) {
		GP_DEBUG(1, "Unknown pixel type\n");
		goto err3;
	}

	if (flags & GP_FB_SHADOW) {
		fb->pixmap.pixels = malloc(fscri.smem_len);

		if (!fb->pixmap.pixels) {
			GP_DEBUG(1, "Malloc failed :(");
			goto err3;
		}
	}

	fb->fb_mem = mmap(NULL, fscri.smem_len,
	                  PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, fd, 0);

	if (fb->fb_mem == MAP_FAILED) {
		GP_DEBUG(1, "mmaping framebuffer failed: %s", strerror(errno));
		goto err4;
	}

	fb->fb_fd = fd;
	fb->bsize = fscri.smem_len;
	strcpy(fb->path, path);
	fb->flags = flags;

	if (!(flags & GP_FB_SHADOW))
		fb->pixmap.pixels = fb->fb_mem;

	fb->pixmap.w = vscri.xres;
	fb->pixmap.h = vscri.yres;

	fb->pixmap.axes_swap = 0;
	fb->pixmap.x_swap    = 0;
	fb->pixmap.y_swap    = 0;

	fb->pixmap.bpp = vscri.bits_per_pixel;
	fb->pixmap.bytes_per_row  = fscri.line_length;
	fb->pixmap.pixel_type = pixel_type;

	int shadow = flags & GP_FB_SHADOW;
	int kbd = flags & GP_FB_INPUT_KBD;

	/* update API */
	backend->name = "Linux FB";
	backend->pixmap = &fb->pixmap;
	backend->flip = shadow ? fb_flip_shadow : NULL;
	backend->update_rect = shadow ? fb_update_rect_shadow : NULL;
	backend->set_attr = NULL;
	backend->resize_ack = NULL;
	backend->poll = kbd ? fb_poll : NULL;
	backend->wait = kbd ? fb_wait : NULL;
	backend->exit = fb_exit;
	backend->fd = fb->con_fd;
	backend->timers = NULL;

	gp_event_queue_init(&backend->event_queue, vscri.xres, vscri.yres, 0);

	return backend;
err4:
	if (flags & GP_FB_SHADOW)
		free(fb->pixmap.pixels);
err3:
	close(fd);
err2:
	if (flags & GP_FB_INPUT_KBD)
		exit_kbd(fb);
err1:
	free_console(fb);
err0:
	free(backend);
	return NULL;
}
