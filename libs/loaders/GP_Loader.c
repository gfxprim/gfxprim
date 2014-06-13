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
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

/*

  General functions for loading and saving bitmaps.

 */

#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>

#include "core/GP_Debug.h"

#include "loaders/GP_Loaders.h"
#include "loaders/GP_Loader.h"

#define MAX_LOADERS 64

static GP_Loader *loaders[MAX_LOADERS] = {
	&GP_JPG,
	&GP_PNG,
	&GP_TIFF,
	&GP_GIF,
	&GP_BMP,
	&GP_PBM,
	&GP_PGM,
	&GP_PPM,
	&GP_PNM,
	&GP_JP2,
	&GP_PCX,
	&GP_PSP,
	&GP_PSD,
};

static unsigned int get_last_loader(void)
{
	unsigned int i;

	for (i = 0; i < MAX_LOADERS; i++) {
		if (!loaders[i])
			return i ? i - 1 : 0;
	}

	return i - 1;
}

int GP_LoaderRegister(GP_Loader *self)
{
	unsigned int i;

	GP_DEBUG(1, "Registering loader for '%s'", self->fmt_name);

	/* We have to keep the last terminating NULL */
	for (i = 0; i < MAX_LOADERS - 2; i++) {
		if (loaders[i] == self) {
			GP_DEBUG(1, "Loader '%s' allready registered",
			         self->fmt_name);
			errno = EEXIST;
			return 1;
		}

		if (!loaders[i])
			break;
	}

	if (loaders[i]) {
		GP_DEBUG(1, "Loaders table is full");
		errno = ENOSPC;
		return 1;
	}

	loaders[i] = self;

	return 0;
}

void GP_LoaderUnregister(GP_Loader *self)
{
	unsigned int i, last = get_last_loader();

	if (self == NULL)
		return;

	GP_DEBUG(1, "Unregistering loader for '%s'", self->fmt_name);

	for (i = 0; loaders[i]; i++) {
		if (loaders[i] == self) {
			loaders[i] = loaders[last];
			loaders[last] = NULL;
			return;
		}
	}

	GP_WARN("Loader '%s' (%p) wasn't registered", self->fmt_name, self);
}

void GP_ListLoaders(void)
{
	unsigned int i, j;

	for (i = 0; loaders[i]; i++) {
		printf("Format: %s\n", loaders[i]->fmt_name);
		printf("Read:\t%s\n", loaders[i]->Read ? "Yes" : "No");
		printf("Save:\t%s\n", loaders[i]->Save ? "Yes" : "No");
		if (loaders[i]->save_ptypes) {
			printf("Saves Pixel Types: ");
			for (j = 0; loaders[i]->save_ptypes[j]; j++) {
				GP_PixelType ptype = loaders[i]->save_ptypes[j];
				printf("%s ", GP_PixelTypeName(ptype));
			}
			printf("\n");
		}
		printf("Match:\t%s\n", loaders[i]->Match ? "Yes" : "No");
		printf("Extensions: ");
		for (j = 0; loaders[i]->extensions[j]; j++)
			printf("%s ", loaders[i]->extensions[j]);
		printf("\n");

		if (loaders[i+1] != NULL)
			printf("\n");
	}
}

static struct GP_Loader *loader_by_extension(const char *ext)
{
	unsigned int i, j;

	for (i = 0; loaders[i]; i++) {
		for (j = 0; loaders[i]->extensions[j] != NULL; j++) {
			if (!strcasecmp(ext, loaders[i]->extensions[j])) {
				GP_DEBUG(1, "Found loader '%s'",
				         loaders[i]->fmt_name);
				return loaders[i];
			}
		}
	}

	return NULL;
}

static const char *get_ext(const char *path)
{
	size_t len = strlen(path);
	int i;

	for (i = len - 1; i >= 0; i--)
		if (path[i] == '.')
			break;

	if (path[i] != '.')
		return NULL;

	return path + i + 1;
}

static struct GP_Loader *loader_by_filename(const char *path)
{
	const char *ext = get_ext(path);

	if (ext == NULL)
		return NULL;

	GP_DEBUG(1, "Loading file by filename extension '%s'", ext);

	return loader_by_extension(ext);
}

static const GP_Loader *loader_by_signature(const char *path)
{
	uint8_t buf[32];
	FILE *f;
	int err;

	const GP_Loader *ret;

	GP_DEBUG(1, "Trying to load by file signature");

	f = fopen(path, "rb");

	if (f == NULL) {
		err = errno;
		GP_DEBUG(1, "Failed to open file '%s'", path);
		goto err0;
	}

	if (fread(buf, sizeof(buf), 1, f) < 1) {
		GP_DEBUG(1, "Failed to read start of the file '%s'", path);
		err = EIO;
		goto err1;
	}

	fclose(f);

	ret = GP_MatchSignature(buf);

	if (ret == NULL)
		errno = ENOSYS;

	return ret;
err1:
	fclose(f);
err0:
	errno = err;
	return NULL;
}

GP_Context *GP_ReadImage(GP_IO *io, GP_ProgressCallback *callback)
{
	char buf[32];
	off_t start;
	const GP_Loader *loader;

	start = GP_IOTell(io);
	if (start == (off_t)-1) {
		GP_DEBUG(1, "Failed to get IO stream offset: %s",
		         strerror(errno));
		return NULL;
	}

	if (GP_IOFill(io, buf, sizeof(buf))) {
		GP_DEBUG(1, "Failed to read first 32 bytes: %s",
		         strerror(errno));
		return NULL;
	}

	if (GP_IOSeek(io, start, GP_IO_SEEK_SET) != start) {
		GP_DEBUG(1, "Failed to seek at the start of the stream: %s",
		         strerror(errno));
		return NULL;
	}

	loader = GP_MatchSignature(buf);

	if (!loader) {
		GP_DEBUG(1, "Failed to find a loader by signature for"
		            "(%x (%c) %x (%c)...)",
			    buf[0], isprint(buf[0]) ? buf[0] : ' ',
			    buf[1], isprint(buf[1]) ? buf[1] : ' ');
		errno = ENOSYS;
		return NULL;
	}

	if (!loader->Read) {
		GP_DEBUG(1, "Loader for '%s' does not support reading",
		         loader->fmt_name);
		errno = ENOSYS;
		return NULL;
	}

	return loader->Read(io, callback);
}

GP_Context *GP_LoaderLoadImage(const GP_Loader *self, const char *src_path,
                               GP_ProgressCallback *callback)
{
	GP_IO *io;
	GP_Context *res;
	int err;

	if (!self->Read) {
		errno = ENOSYS;
		return NULL;
	}

	io = GP_IOFile(src_path, GP_IO_RDONLY);
	if (!io)
		return NULL;

	res = self->Read(io, callback);

	err = errno;
	GP_IOClose(io);
	errno = err;

	return res;
}

GP_Context *GP_LoadImage(const char *src_path, GP_ProgressCallback *callback)
{
	int err;
	struct stat st;

	if (access(src_path, R_OK)) {
		err = errno;
		GP_DEBUG(1, "Failed to access file '%s' : %s",
		            src_path, strerror(errno));
		errno = err;
		return NULL;
	}

	if (stat(src_path, &st)) {
		GP_WARN("Failed to stat '%s': %s", src_path, strerror(errno));
	} else {
		if (st.st_mode & S_IFDIR) {
			errno = EISDIR;
			return NULL;
		}
	}

	GP_Context *img;
	const GP_Loader *ext_load = NULL, *sig_load;

	ext_load = loader_by_filename(src_path);

	if (ext_load) {
		img = GP_LoaderLoadImage(ext_load, src_path, callback);

		if (img)
			return img;
	}

	sig_load = loader_by_signature(src_path);

	/*
	 * Avoid further work if extension matches the signature but image
	 * couldn't be loaded. Probably unimplemented format or damaged file.
	 */
	if (ext_load == sig_load)
		return NULL;

	if (ext_load && sig_load) {
		GP_WARN("File '%s': Extension says %s but signature %s",
			src_path, ext_load->fmt_name, sig_load->fmt_name);
	}

	if (sig_load)
		return GP_LoaderLoadImage(sig_load, src_path, callback);

	errno = ENOSYS;
	return NULL;
}

int GP_LoadMetaData(const char *src_path, GP_MetaData *data)
{
	const char *ext;
	int err;

	if (access(src_path, R_OK)) {
		err = errno;
		GP_DEBUG(1, "Failed to access file '%s' : %s",
		            src_path, strerror(errno));
		errno = err;
		return 1;
	}

	ext = get_ext(src_path);

	if (ext == NULL)
		goto out;

	if (!strcasecmp(ext, "jpg") || !strcasecmp(ext, "jpeg"))
		return GP_LoadJPGMetaData(src_path, data);

	if (!strcasecmp(ext, "png"))
		return GP_LoadPNGMetaData(src_path, data);

out:
	errno = ENOSYS;
	return 1;
}

int GP_SaveImage(const GP_Context *src, const char *dst_path,
                 GP_ProgressCallback *callback)
{
	struct GP_Loader *l = loader_by_filename(dst_path);

	if (l == NULL) {
		errno = EINVAL;
		return 1;
	}

	if (l->Save)
		return l->Save(src, dst_path, callback);

	errno = ENOSYS;
	return 1;
}

const GP_Loader *GP_MatchSignature(const void *buf)
{
	unsigned int i;

	for (i = 0; loaders[i]; i++) {
		if (loaders[i]->Match && loaders[i]->Match(buf) == 1) {
			GP_DEBUG(1, "Found loader '%s'", loaders[i]->fmt_name);
			return loaders[i];
		}
	}

	GP_DEBUG(1, "Loader not found");

	return NULL;
}

const GP_Loader *GP_MatchExtension(const char *path)
{
	return loader_by_filename(path);
}
