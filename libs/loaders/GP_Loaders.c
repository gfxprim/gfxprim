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

/*

  General functions for loading and saving bitmaps.
  
 */

#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "core/GP_Debug.h"

#include "GP_Loaders.h"

enum GP_ImageFmt {
	GP_FMT_UNKNOWN,
	GP_FMT_PNG,
	GP_FMT_JPG,
	GP_FMT_BMP,
	GP_FMT_GIF,
	GP_FMT_PBM,
	GP_FMT_PGM,
	GP_FMT_PPM,
};

enum GP_ImageFmt filename_to_fmt(const char *path)
{
	size_t len = strlen(path);

	if (len < 3)
		return GP_FMT_UNKNOWN;

	switch (path[len - 1]) {
	/* PNG, JPG, JPEG */
	case 'g':
	case 'G':
		switch (path[len - 2]) {
		case 'n':
		case 'N':
			if (path[len - 3] == 'p' ||
			    path[len - 3] == 'P')
				return GP_FMT_PNG;
		break;
		case 'p':
		case 'P':
			if (path[len - 3] == 'j' ||
			    path[len - 3] == 'J')
				return GP_FMT_JPG;
		break;
		case 'e':
		case 'E':
			if ((path[len - 3] == 'p' ||
			     path[len - 3] == 'P') &&
			    (path[len - 4] == 'j' ||
			     path[len - 4] == 'J'))
				return GP_FMT_JPG;
		break;
		}
	break;
	/* PPM, PGM, PBM, PNM */
	case 'm':
	case 'M':
		switch (path[len - 2]) {
		case 'b':
		case 'B':
			if (path[len - 3] == 'p' ||
			    path[len - 3] == 'P')
				return GP_FMT_PBM;
		break;
		case 'g':
		case 'G':
			if (path[len - 3] == 'p' ||
			    path[len - 3] == 'P')
				return GP_FMT_PGM;
		break;
		case 'p':
		case 'P':
			if (path[len - 3] == 'p' ||
			    path[len - 3] == 'P')
				return GP_FMT_PPM;
		break;
		}
	break;
	/* BMP */
	case 'P':
	case 'p':
		switch (path[len - 2]) {
		case 'M':
		case 'm':
			if (path[len - 3] == 'B' ||
			    path[len - 3] == 'b')
				return GP_FMT_BMP;
		break;
		}
	break;
	/* GIF */
	case 'F':
	case 'f':
		switch (path[len - 2]) {
		case 'I':
		case 'i':
			if (path[len - 3] == 'G' ||
			    path[len - 3] == 'g')
				return GP_FMT_GIF;
		break;
		}
	break;
	}

	return GP_FMT_UNKNOWN;
}

GP_Context *GP_LoadImage(const char *src_path, GP_ProgressCallback *callback)
{
	int saved_errno;
	
	if (access(src_path, R_OK)) {
		
		saved_errno = errno;

		GP_DEBUG(1, "Failed to access file '%s' : %s",
		            src_path, strerror(errno));
		
		errno = saved_errno;
		
		return NULL;
	}
	
	enum GP_ImageFmt fmt = filename_to_fmt(src_path);

	switch (fmt) {
	case GP_FMT_JPG:
		return GP_LoadJPG(src_path, callback);
	case GP_FMT_PNG:
		return GP_LoadPNG(src_path, callback);
	case GP_FMT_GIF:
		return GP_LoadGIF(src_path, callback);
	case GP_FMT_BMP:
		return GP_LoadGIF(src_path, callback);
	case GP_FMT_PBM:
		return GP_LoadPBM(src_path, callback);
	case GP_FMT_PGM:
		return GP_LoadPGM(src_path, callback);
	case GP_FMT_PPM:
		return GP_LoadPPM(src_path, callback);
	
	case GP_FMT_UNKNOWN:
	break;
	}

	//TODO file signature based check
	errno = ENOSYS;
	return NULL;
}

int GP_LoadMetaData(const char *src_path, GP_MetaData *data)
{
	int saved_errno;
	
	if (access(src_path, R_OK)) {
		
		saved_errno = errno;

		GP_DEBUG(1, "Failed to access file '%s' : %s",
		            src_path, strerror(errno));
		
		errno = saved_errno;
		
		return 1;
	}

	enum GP_ImageFmt fmt = filename_to_fmt(src_path);

	switch (fmt) {
	case GP_FMT_JPG:
		return GP_LoadJPGMetaData(src_path, data);
	case GP_FMT_PNG:
		return GP_LoadPNGMetaData(src_path, data);
	
	default:
	break;
	}

	errno = ENOSYS;
	return 1;
}

int GP_SaveImage(const GP_Context *src, const char *dst_path,
                 GP_ProgressCallback *callback)
{
	enum GP_ImageFmt fmt = filename_to_fmt(dst_path);

	switch (fmt) {
	case GP_FMT_JPG:
		return GP_SaveJPG(src, dst_path, callback);
	case GP_FMT_PNG:
		return GP_SavePNG(src, dst_path, callback);

	default:
	break;
	}

	errno = ENOSYS;
	return 1;
}
