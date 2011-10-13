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
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#include <linux/fb.h>
#include <linux/kd.h>
#include <linux/vt.h>

#include "core/GP_Debug.h"
#include "GP_Framebuffer.h"

/*
 * Allocates and switches to newly allocated console.
 */
static int allocate_console(struct GP_Framebuffer *fb)
{
	struct vt_stat vts;
	int fd, nr;
	char buf[255];

	/* allocate and switch to new console */
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

	close(fd);

	snprintf(buf, sizeof(buf), "/dev/tty%i", nr);
	fd = open(buf, O_RDWR);

	if (fd < 0) {
		GP_DEBUG(1, "Opening console %s failed: %s",
		            buf, strerror(errno));
		close(fd);
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
	}
	
	fb->con_nr = nr;
	fb->con_fd = fd;

	return 0;
}

GP_Framebuffer *GP_FramebufferInit(const char *path)
{
	GP_Framebuffer *fb = malloc(sizeof (GP_Framebuffer) + strlen(path) + 1);
	struct fb_fix_screeninfo fscri;
	struct fb_var_screeninfo vscri;
	int fd;

	if (fb == NULL)
		return NULL;

	if (allocate_console(fb))
		goto err1;

	/* open and mmap framebuffer */
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

	fb->context.w = vscri.xres;
	fb->context.h = vscri.yres;
	
	fb->context.axes_swap = 0;
	fb->context.x_swap    = 0;
	fb->context.y_swap    = 0;

	fb->context.bpp = vscri.bits_per_pixel;
	fb->context.bytes_per_row  = fscri.line_length;
	fb->context.pixel_type = pixel_type;

	return fb;
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
	free(fb);
	return NULL;
}

void GP_FramebufferExit(GP_Framebuffer *fb)
{
	/* unmap framebuffer */
	munmap(fb->context.pixels, fb->bsize);
	close(fb->fb_fd);
	
	/* reset keyboard */
	ioctl(fb->con_fd, KDSETMODE, KD_TEXT);
	
	/* switch back console */
	if (fb->last_con_nr != -1)
		ioctl(fb->con_fd, VT_ACTIVATE, fb->last_con_nr);
	
	close(fb->con_fd);
	free(fb);
}
