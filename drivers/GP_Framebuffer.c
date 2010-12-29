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

#include "GP_Pixel.h"

#include "GP_Framebuffer.h"

GP_Framebuffer *GP_FramebufferInit(const char *path)
{
	GP_Framebuffer *fb = malloc(sizeof (GP_Framebuffer) + strlen(path) + 1);
	int fd;
	struct fb_fix_screeninfo fscri;
	struct fb_var_screeninfo vscri;

	if (fb == NULL)
		return NULL;

	fd = open(path, O_RDWR);

	if (fd < 0) {
		perror("opening framebuffer failed");
		free(fb);
		return NULL;
	}

	if (ioctl(fd, FBIOGET_FSCREENINFO, &fscri) < 0) {
		perror("ioctl FBIOGET_FSCREENINFO");
		free(fb);
		close(fd);
		return NULL;
	}
	
	if (ioctl(fd, FBIOGET_VSCREENINFO, &vscri) < 0) {
		perror("ioctl FBIOGET_VSCREENINFO");
		free(fb);
		close(fd);
		return NULL;
	}

	fb->context.pixels = mmap(NULL, fscri.smem_len,
	                          PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED,
				  fd, 0);

	if (fb->context.pixels == MAP_FAILED) {
		perror("mmaping framebuffer failed");
		free(fb);
		close(fd);
		return NULL;
	}

	fb->fd    = fd;
	fb->bsize = fscri.smem_len;
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

	fb->context.bits_per_pixel = vscri.bits_per_pixel;
	fb->context.bytes_per_row  = fscri.line_length;

	fb->context.pixel_type = GP_PIXEL_XRGB8888;

	return fb;
}

void GP_FramebufferExit(GP_Framebuffer *fb)
{
	munmap(fb->context.pixels, fb->bsize);
	close(fb->fd);
	free(fb);
}
