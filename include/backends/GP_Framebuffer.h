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

#ifndef GP_FRAMEBUFFER_H
#define GP_FRAMEBUFFER_H

#include "core/GP_Context.h"

typedef struct GP_Framebuffer {
	GP_Context context;
	uint32_t bsize;
	int con_fd;
	int con_nr;
	int last_con_nr;
	int fb_fd;
	char path[];
} GP_Framebuffer;

/*
 * Initalize framebuffer.
 */
GP_Framebuffer *GP_FramebufferInit(const char *path);

/*
 * Deinitalize framebuffer.
 */
void GP_FramebufferExit(GP_Framebuffer *fb);

#endif /* GP_FRAMEBUFFER_H */
