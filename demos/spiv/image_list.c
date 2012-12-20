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
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include <stdlib.h>

#include <core/GP_Debug.h>

#include "image_list.h"

struct image_list {
	/* list we got from the app */
	const char **args;
	unsigned int cur_arg;
	unsigned int max_arg;

	/* path to the currently loaded image */
	char path[1024];
	int path_loaded:1;
};

static void next_img(struct image_list *self)
{
	if (++self->cur_arg == self->max_arg)
		self->cur_arg = 0;

	self->path_loaded = 0;
}

static void prev_img(struct image_list *self)
{
	if (self->cur_arg == 0)
		self->cur_arg = self->max_arg - 1;
	else
		self->cur_arg--;

	self->path_loaded = 0;
}

static void load_path(struct image_list *self)
{
	snprintf(self->path, sizeof(self->path), "%s", self->args[self->cur_arg]);

	self->path_loaded = 1;
}

const char *image_list_move(struct image_list *self, int direction)
{
	GP_DEBUG(2, "Moving list by %i", direction);

	int i;

	for (i = 0; i < direction; i++)
		next_img(self);

	for (i = 0; i > direction; i--)
		prev_img(self);

	return image_list_img_path(self);
}

struct image_list *image_list_create(const char *args[])
{
	struct image_list *self;

	GP_DEBUG(1, "Creating image list");
	
	self = malloc(sizeof(struct image_list));

	if (self == NULL)
		return NULL;

	self->args = args;
	self->cur_arg = 0;
	
	self->path_loaded = 0;

	self->max_arg = 0;
	while (args[++self->max_arg] != NULL);

	return self;
}

const char *image_list_img_path(struct image_list *self)
{
	if (!self->path_loaded)
		load_path(self);
	
	GP_DEBUG(2, "Returning path '%s'", self->path);

	return self->path;
}
