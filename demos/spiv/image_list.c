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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <loaders/GP_Loader.h>
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
	
	/* directory handling */
	int in_dir:1;

	int cur_file;
	int max_file;
	struct dirent **dir_files;
};

static int dir_cmp(const struct dirent **a, const struct dirent **b)
{
	return strcasecmp((*a)->d_name, (*b)->d_name);
}

static int dir_filter(const struct dirent *d)
{
	/* Ignore some filenames */
	if (!strcmp(d->d_name, "."))
		return 0;
	
	if (!strcmp(d->d_name, ".."))
		return 0;
	
	//TODO: filter out directories
	
	if (GP_MatchExtension(d->d_name) == NULL)
		return 0;

	GP_DEBUG(4, "Adding file '%s'", d->d_name);
	
	return 1;
}

static void try_load_dir(struct image_list *self)
{
	struct stat sb;
	const char *path = self->args[self->cur_arg];

	if (stat(path, &sb)) {
		GP_WARN("Failed to stat '%s': %s", path, strerror(errno));
		return;
	}

	if (!(sb.st_mode & S_IFDIR))
		return;

	GP_DEBUG(1, "Loading directory '%s' content.", path);

	int ret = scandir(path, &self->dir_files, dir_filter, dir_cmp);

	if (ret == -1) {
		GP_WARN("Failed to scandir '%s': %s", path, strerror(errno));
		return;
	}

	if (ret == 0) {
		GP_DEBUG(1, "There are no files in '%s'", path);
		return;
	}

	self->max_file = ret;
	self->cur_file = 0;
	self->in_dir = 1;
}

static void exit_dir(struct image_list *self)
{
	int i;

	GP_DEBUG(1, "Freeing directory '%s' content.",
	         self->args[self->cur_arg]);

	for (i = 0; i < self->max_file; i++)
		free(self->dir_files[i]);
	
	free(self->dir_files);

	self->in_dir = 0;
}

static void next_img(struct image_list *self)
{
	if (self->in_dir) {
		if (++self->cur_file == self->max_file) {
			exit_dir(self);
		} else {
			self->path_loaded = 0;
			return;
		}
	}
	
	if (++self->cur_arg == self->max_arg)
		self->cur_arg = 0;
	
	try_load_dir(self);

	self->path_loaded = 0;
}

static void prev_img(struct image_list *self)
{
	if (self->in_dir) {
		if (self->cur_file-- == 0) {
			exit_dir(self);
		} else {
			self->path_loaded = 0;
			return;
		}
	}

	if (self->cur_arg == 0)
		self->cur_arg = self->max_arg - 1;
	else
		self->cur_arg--;

	try_load_dir(self);

	self->path_loaded = 0;
}

static void load_path(struct image_list *self)
{
	if (self->in_dir) {
		//TODO: eliminate double /
		snprintf(self->path, sizeof(self->path), "%s/%s",
		         self->args[self->cur_arg],
		         self->dir_files[self->cur_file]->d_name);

	} else {
		snprintf(self->path, sizeof(self->path), "%s",
		         self->args[self->cur_arg]);
	}

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
	
	self->dir_files = 0;
	self->in_dir = 0;

	self->max_arg = 0;
	while (args[++self->max_arg] != NULL);

	try_load_dir(self);

	return self;
}

const char *image_list_img_path(struct image_list *self)
{
	if (!self->path_loaded)
		load_path(self);
	
	GP_DEBUG(2, "Returning path '%s'", self->path);

	return self->path;
}
