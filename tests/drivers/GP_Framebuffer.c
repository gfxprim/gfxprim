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
 * Copyright (C) 2009-2010 Jiri "BlueBear" Dluhos                            *
 *                         <jiri.bluebear.dluhos@gmail.com>                  *
 *                                                                           *
 * Copyright (C) 2009-2010 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>

#include <linux/fb.h>
#include <linux/kd.h>
#include <linux/vt.h>

#include "GP_Pixel.h"

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
		perror("opening console failed");
		return -1;
	}
	
	if (ioctl(fd, VT_OPENQRY, &nr) < 0) {
		perror("ioctl VT_OPENQRY");
		close(fd);
		return -1;
	}

	close(fd);

	snprintf(buf, sizeof(buf), "/dev/tty%i", nr);
	fd = open(buf, O_RDWR);

	if (fd < 0) {
		perror("opening console failed");
		close(fd);
		return -1;
	}
	
	if (ioctl(fd, VT_GETSTATE, &vts) == 0)
		fb->last_con_nr = vts.v_active;
	else	
		fb->last_con_nr = -1;

	if (ioctl(fd, VT_ACTIVATE, nr) < 0) {
		perror("ioctl VT_ACTIVATE");
		close(fd);
		return -1;
	}

	if (ioctl(fd, VT_WAITACTIVE, nr) < 0) {
		perror("ioctl VT_WAITACTIVE");
		close(fd);
		return -1;
	}
	
	/* turn off blinking cursor */
	if (ioctl(fd, KDSETMODE, KD_GRAPHICS) < 0) {
		perror("ioctl KDSETMODE");
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
		perror("opening framebuffer failed");
		goto err2;
	}

	if (ioctl(fd, FBIOGET_FSCREENINFO, &fscri) < 0) {
		perror("ioctl FBIOGET_FSCREENINFO");
		goto err3;
	}
	
	if (ioctl(fd, FBIOGET_VSCREENINFO, &vscri) < 0) {
		perror("ioctl FBIOGET_VSCREENINFO");
		goto err3;
	}

	fb->context.pixels = mmap(NULL, fscri.smem_len,
	                          PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED,
				  fd, 0);

	if (fb->context.pixels == MAP_FAILED) {
		perror("mmaping framebuffer failed");
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

	fb->context.clip_w_min = 0;
	fb->context.clip_h_min = 0;
	fb->context.clip_w_max = fb->context.w - 1;
	fb->context.clip_h_max = fb->context.h - 1;

	fb->context.bpp = vscri.bits_per_pixel;
	fb->context.bytes_per_row  = fscri.line_length;

	fb->context.pixel_type = GP_PIXEL_RGB565;

	return fb;
err3:
	close(fd);
err2:
	close(fb->con_fd);
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
