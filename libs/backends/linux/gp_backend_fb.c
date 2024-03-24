// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2023 Cyril Hrubis <metan@ucw.cz>
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
#include <core/gp_pixmap.h>
#include <input/gp_input_driver_kbd.h>
#include <backends/gp_linux_input.h>
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
	/* hook for the keymap to turn on/off keyboard leds */
	int leds;
	gp_ev_feedback feedback;
	/* queue for input events */
	gp_ev_queue ev_queue;
	/* poll fd struct */
	gp_fd fd;

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

	if (ioctl(fb->con_fd, KDSETLED, 0))
		GP_WARN("Failed to set kbd leds %s", strerror(errno));
}

static int leds_to_flags(int leds)
{
	int ret = 0;

	if (leds & GP_KBD_LED_NUM_LOCK)
		ret |= LED_NUM;

	if (leds & GP_KBD_LED_CAPS_LOCK)
		ret |= LED_CAP;

	if (leds & GP_KBD_LED_SCROLL_LOCK)
		ret |= LED_SCR;

	return ret;
}

static int set_get_kbd_leds(gp_ev_feedback *self, gp_ev_feedback_op *op)
{
	struct fb_priv *fb_priv = GP_CONTAINER_OF(self, struct fb_priv, feedback);

	switch (op->op) {
	case GP_EV_LEDS_ON:
		fb_priv->leds |= op->val;
	break;
	case GP_EV_LEDS_OFF:
		fb_priv->leds &= ~(op->val);
	break;
	case GP_EV_LEDS_GET:
		op->val = fb_priv->leds;
		return 0;
	break;
	}

	if (ioctl(fb_priv->con_fd, KDSETLED, leds_to_flags(fb_priv->leds)))
		return -1;

	return 0;
}

static enum gp_poll_event_ret fb_process_fd(gp_fd *self)
{
	gp_backend *backend = self->priv;
	struct fb_priv *fb = GP_BACKEND_PRIV(backend);
	unsigned char buf[16];
	int i, res;

	res = read(fb->con_fd, buf, sizeof(buf));

	for (i = 0; i < res; i++)
		gp_input_driver_kbd_event_put(backend->event_queue, buf[i]);

	return 0;
}

static int set_kbd(struct fb_priv *fb, int mode)
{
	if (ioctl(fb->con_fd, KDGKBMODE, &fb->saved_kb_mode)) {
		GP_DEBUG(1, "Failed to ioctl KDGKBMODE tty%i: %s",
		         fb->con_nr, strerror(errno));
		close(fb->con_fd);
		return -1;
	}

	GP_DEBUG(2, "Previous keyboard mode was '%i'",
	         fb->saved_kb_mode);

	if (ioctl(fb->con_fd, KDSKBMODE, mode) < 0) {
		GP_DEBUG(1, "Failed to ioctl KDSKBMODE tty%i: %s",
		        fb->con_nr, strerror(errno));
		close(fb->con_fd);
		return -1;
	}

	return 0;
}

static int init_kbd(gp_backend *backend, struct fb_priv *fb)
{
	struct termios t;

	fb->fd = (gp_fd) {
		.fd = fb->con_fd,
		.event = fb_process_fd,
		.events = GP_POLLIN,
		.priv = &backend,
	};

	if (gp_poll_add(&backend->fds, &fb->fd)) {
		close(fb->con_fd);
		return -1;
	}

	if (tcgetattr(fb->con_fd, &fb->ts)) {
		GP_WARN("Failed to tcgetattr(): %s", strerror(errno));
		fb->restore_termios = 0;
	} else {
		fb->restore_termios = 1;
	}

	cfmakeraw(&t);

	if (tcsetattr(fb->con_fd, TCSANOW, &t) < 0) {
		GP_DEBUG(1, "Failed to tcsetattr(): %s",
		         strerror(errno));
		close(fb->con_fd);
		return -1;
	}

	if (set_kbd(fb, K_MEDIUMRAW))
		return -1;

	return 0;
}

static int disable_kbd(struct fb_priv *fb)
{
	if (set_kbd(fb, K_OFF))
		return -1;

	return 0;
}

static int setup_kbd(gp_backend *backend, struct fb_priv *fb)
{
	if (fb->flags & GP_FB_INPUT_KBD)
		return init_kbd(backend, fb);

	return disable_kbd(fb);
}

/*
 * Allocates and switches to newly allocated console.
 */
static int allocate_console(struct fb_priv *fb, int flags)
{
	struct vt_stat vts;
	int fd, nr = -1;
	char buf[255];
	const char *tty = "/dev/tty1";

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

	snprintf(buf, sizeof(buf), "/dev/tty%i", fb->last_con_nr);

	if (flags & GP_FB_ALLOC_CON) {
		if (ioctl(fd, VT_OPENQRY, &nr) < 0) {
			GP_DEBUG(1, "Failed to ioctl VT_OPENQRY /dev/tty1: %s",
			            strerror(errno));
			close(fd);
			return -1;
		}

		GP_DEBUG(1, "Has been granted tty%i", nr);

		snprintf(buf, sizeof(buf), "/dev/tty%i", nr);
		tty = buf;
	}

	close(fd);

	fd = open(tty, O_RDWR | O_NONBLOCK);

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

static void fb_exit(gp_backend *self)
{
	struct fb_priv *fb = GP_BACKEND_PRIV(self);

	if (fb->flags & GP_FB_SHADOW)
		free(fb->pixmap.pixels);

	/* unmap framebuffer */
	munmap(fb->fb_mem, fb->bsize);
	close(fb->fb_fd);

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

	size_t bpp = gp_pixel_size(fb->pixmap.pixel_type);
	size_t size = ((x1 - x0 + 1) * bpp) / 8;

	for (;y0 <= y1; y0++) {
		void *src = GP_PIXEL_ADDR(&fb->pixmap, x0, y0);
		void *dst = (char*)fb->fb_mem +
                            y0 * fb->pixmap.bytes_per_row +
                            (x0 * bpp)/8;
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

	if ((flags & GP_FB_INPUT_LINUX) && (flags & GP_FB_INPUT_KBD)) {
		GP_FATAL("Linux input and console KBD cannot be both enabled!");
		return NULL;
	}

	size_t size = sizeof(gp_backend) +
	              sizeof(struct fb_priv) + strlen(path) + 1;

	backend = malloc(size);
	if (!backend)
		return NULL;

	memset(backend, 0, size);

	fb = GP_BACKEND_PRIV(backend);
	fb->flags = flags;

	if (allocate_console(fb, flags))
		goto err0;

	if (setup_kbd(backend, fb))
		goto err1;

	if (flags & GP_FB_INPUT_LINUX) {
		if (gp_linux_input_hotplug_new(backend))
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

	if (!(flags & GP_FB_SHADOW))
		fb->pixmap.pixels = fb->fb_mem;

	fb->pixmap.w = vscri.xres;
	fb->pixmap.h = vscri.yres;

	fb->pixmap.axes_swap = 0;
	fb->pixmap.x_swap    = 0;
	fb->pixmap.y_swap    = 0;

	fb->pixmap.bytes_per_row  = fscri.line_length;
	fb->pixmap.pixel_type = pixel_type;

	fb->feedback.set_get = set_get_kbd_leds;

	int shadow = flags & GP_FB_SHADOW;
	int kbd = flags & GP_FB_INPUT_KBD;

	/* update API */
	backend->name = "Linux FB";
	backend->pixmap = &fb->pixmap;
	backend->flip = shadow ? fb_flip_shadow : NULL;
	backend->update_rect = shadow ? fb_update_rect_shadow : NULL;
	backend->exit = fb_exit;
	backend->event_queue = &fb->ev_queue;

	gp_ev_queue_init(backend->event_queue, vscri.xres, vscri.yres, 0, GP_EVENT_QUEUE_LOAD_KEYMAP);

	if (kbd)
		gp_ev_queue_feedback_register(backend->event_queue, &fb->feedback);

	return backend;
err4:
	if (flags & GP_FB_SHADOW)
		free(fb->pixmap.pixels);
err3:
	close(fd);
err2:
	exit_kbd(fb);

	if (flags & GP_FB_INPUT_LINUX)
		gp_backend_input_destroy(backend);
err1:
	free_console(fb);
err0:
	free(backend);
	return NULL;
}
