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

/*

  General functions for loading bitmaps.
  
 */

#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "core/GP_Debug.h"

#include "GP_Loaders.h"

GP_Context *GP_LoadImage(const char *src_path, GP_ProgressCallback *callback)
{
	int len, saved_errno;
	GP_Context *res = NULL;
	
	if (access(src_path, R_OK)) {
		
		saved_errno = errno;

		GP_DEBUG(1, "Failed to access file '%s' : %s",
		            src_path, strerror(errno));
		
		errno = saved_errno;
		
		return NULL;
	}
	
	len = strlen(src_path);

	if (len < 3)
		goto skip_filename_check;

	switch (src_path[len - 1]) {
	/* PNG, JPG, JPEG */
	case 'g':
	case 'G':
		switch (src_path[len - 2]) {
		case 'n':
		case 'N':
			if (src_path[len - 3] == 'p' ||
			    src_path[len - 3] == 'P')
				GP_LoadPNG(src_path, &res, callback);
		break;
		case 'p':
		case 'P':
			if (src_path[len - 3] == 'j' ||
			    src_path[len - 3] == 'J')
				GP_LoadJPG(src_path, &res, callback);
		break;
		case 'e':
		case 'E':
			if ((src_path[len - 3] == 'p' ||
			     src_path[len - 3] == 'P') &&
			    (src_path[len - 4] == 'j' ||
			     src_path[len - 4] == 'J'))
				GP_LoadJPG(src_path, &res, callback);
		break;
		}
	break;
	/* PPM, PGM, PBM, PNM */
	case 'm':
	case 'M':
		switch (src_path[len - 2]) {
		case 'b':
		case 'B':
			if (src_path[len - 3] == 'p' ||
			    src_path[len - 3] == 'P')
				GP_LoadPBM(src_path, &res);
		break;
		case 'g':
		case 'G':
			if (src_path[len - 3] == 'p' ||
			    src_path[len - 3] == 'P')
				GP_LoadPGM(src_path, &res);
		break;
		case 'p':
		case 'P':
			if (src_path[len - 3] == 'p' ||
			    src_path[len - 3] == 'P')
				GP_LoadPPM(src_path, &res);
		break;
		}
	break;
	/* BMP */
	case 'P':
	case 'p':
		switch (src_path[len - 2]) {
		case 'M':
		case 'm':
			if (src_path[len - 3] == 'B' ||
			    src_path[len - 3] == 'b')
				res = GP_LoadBMP(src_path, callback);
		break;
		}
	break;
	}

skip_filename_check:

	//TODO file signature based check

	return res;
}
