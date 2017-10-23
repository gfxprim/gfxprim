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
	/* counters for files in corresponding arg */
	int *arg_file_counts;

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

	if (!GP_LoaderByFilename(d->d_name))
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

	if (!(sb.st_mode & S_IFDIR)) {
		self->arg_file_counts[self->cur_arg] = 1;
		return;
	}

	GP_DEBUG(1, "Loading directory '%s' content.", path);

	int ret = scandir(path, &self->dir_files, dir_filter, dir_cmp);

	if (ret == -1) {
		GP_WARN("Failed to scandir '%s': %s", path, strerror(errno));
		return;
	}

	if (self->arg_file_counts[self->cur_arg] != ret) {
		GP_DEBUG(1, "Updating arg counter to %i", ret);
		self->arg_file_counts[self->cur_arg] = ret;
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

	/* If we are at first image -> wrap around argv */
	if (self->cur_arg == 0)
		self->cur_arg = self->max_arg - 1;
	else
		self->cur_arg--;

	try_load_dir(self);

	/* if in directory, select last image in it */
	if (self->in_dir)
		self->cur_file = self->max_file - 1;

	self->path_loaded = 0;
}

/*
 * Sets current image, if we are in directory.
 */
static void set_dir_cur_img(struct image_list *self, int img)
{
	if (!self->in_dir) {
		GP_BUG("Not in directory at %s",
		       self->args[self->cur_arg]);
		return;
	}

	if (img < 0 || img >= self->max_file) {
		GP_BUG("Invalid image index %i", img);
		return;
	}

	/* allready there */
	if (self->cur_file == img)
		return;

	self->cur_file = img;
	self->path_loaded = 0;
}

/*
 * Returns current argument from arg list we are in.
 *
 * Either it's image file or directory.
 */
static const char *cur_arg(struct image_list *self)
{
	return self->args[self->cur_arg];
}

/*
 * Sets current argument from arg list.
 */
static void set_cur_arg(struct image_list *self, int arg)
{
	if (arg < 0 || arg >= (int)self->max_arg) {
		GP_BUG("Invalid argument index %i", arg);
		return;
	}

	if (self->in_dir) {
		if ((int)self->cur_arg != arg) {
			exit_dir(self);
			self->cur_arg = arg;
			self->path_loaded = 0;
			try_load_dir(self);
		} else {
			set_dir_cur_img(self, 0);
		}
		return;
	}

	if ((int)self->cur_arg == arg)
		return;

	self->cur_arg = arg;
	self->path_loaded = 0;

	try_load_dir(self);
}

static void load_path(struct image_list *self)
{
	if (self->in_dir) {
		//TODO: eliminate double /
		snprintf(self->path, sizeof(self->path), "%s/%s",
		         cur_arg(self),
			 self->dir_files[self->cur_file]->d_name);

	} else {
		snprintf(self->path, sizeof(self->path), "%s", cur_arg(self));
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

const char *image_list_dir_move(struct image_list *self, int direction)
{
	if (!self->in_dir) {
		GP_DEBUG(2, "Not in directory");
		return image_list_move(self, direction);
	}

	if (direction > 0) {
		if (self->cur_file == self->max_file - 1) {
			GP_DEBUG(2, "Moving after dir '%s'", cur_arg(self));
			next_img(self);
		} else {
			GP_DEBUG(2, "Moving to last image in dir '%s'",
			         cur_arg(self));
			set_dir_cur_img(self, self->max_file - 1);
		}
	} else {
		if (self->cur_file == 0) {
			GP_DEBUG(2, "Moving before dir '%s'", cur_arg(self));
			prev_img(self);
		} else {
			GP_DEBUG(2, "Moving to first image in dir '%s'",
			         cur_arg(self));
			set_dir_cur_img(self, 0);
		}
	}

	return image_list_img_path(self);
}

const char *image_list_first(struct image_list *self)
{
	GP_DEBUG(2, "Moving to the first image in the list");

	set_cur_arg(self, 0);

	if (self->in_dir)
		set_dir_cur_img(self, 0);

	return image_list_img_path(self);
}

const char *image_list_last(struct image_list *self)
{
	GP_DEBUG(2, "Moving to the last image in the list");

	set_cur_arg(self, self->max_arg - 1);

	if (self->in_dir)
		set_dir_cur_img(self, self->max_file - 1);

	return image_list_img_path(self);
}

static unsigned int count_img_to(struct image_list *self, unsigned int arg_to)
{
	unsigned int cur_arg = self->cur_arg;
	unsigned int cur_file = self->cur_file;
	unsigned int count = 0, i;

	for (i = 0; i < arg_to; i++) {
		/* cache number of images in arg */
		if (self->arg_file_counts[i] == -1)
			set_cur_arg(self, i);

		/*
		 * if the counter is still at -1
		 * directory couldn't be loaded
		 */
		if (self->arg_file_counts[i] != -1)
			count += self->arg_file_counts[i];
	}

	/* restore the original position */
	set_cur_arg(self, cur_arg);

	if (self->in_dir)
		set_dir_cur_img(self, cur_file);

	return count;
}

unsigned int image_list_count(struct image_list *self)
{
	return count_img_to(self, self->max_arg);
}

unsigned int image_list_pos(struct image_list *self)
{
	if (!self->in_dir)
		return count_img_to(self, self->cur_arg);

	return count_img_to(self, self->cur_arg) +  self->cur_file;
}

unsigned int image_list_dir_count(struct image_list *self)
{
	if (!self->in_dir)
		return 0;

	return self->max_file;
}

unsigned int image_list_dir_pos(struct image_list *self)
{
	if (!self->in_dir)
		return 0;

	return self->cur_file;
}

struct image_list *image_list_create(const char *args[])
{
	struct image_list *self;
	size_t file_count_size;
	unsigned int i;

	GP_DEBUG(1, "Creating image list");

	self = malloc(sizeof(struct image_list));

	if (self == NULL) {
		GP_WARN("Malloc failed");
		return NULL;
	}

	self->args = args;
	self->cur_arg = 0;

	self->path_loaded = 0;

	self->dir_files = 0;
	self->in_dir = 0;

	self->max_arg = 0;
	while (args[++self->max_arg] != NULL);

	file_count_size = self->max_arg * sizeof(int);
	self->arg_file_counts = malloc(file_count_size);

	if (self->arg_file_counts == NULL) {
		GP_WARN("Malloc failed");
		free(self);
		return NULL;
	}

	for (i = 0; i < self->max_arg; i++)
		self->arg_file_counts[i] = -1;

	try_load_dir(self);

	return self;
}

void image_list_destroy(struct image_list *self)
{
	if (self->in_dir)
		exit_dir(self);

	free(self->arg_file_counts);
	free(self);
}

const char *image_list_img_path(struct image_list *self)
{
	if (!self->path_loaded)
		load_path(self);

	GP_DEBUG(2, "Returning path '%s'", self->path);

	return self->path;
}
