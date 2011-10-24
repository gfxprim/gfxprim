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

  */

#include <math.h>

#include <GP.h>
#include <input/GP_InputDriverLinux.h>

int main(int argc, char *argv[])
{
	struct GP_InputDriverLinux *drv;

	GP_SetDebugLevel(10);

	if (argc != 2) {
		printf("Usage: %s /dev/input/eventX\n", argv[0]);
		return 1;
	}

	drv = GP_InputDriverLinuxOpen(argv[1]);

	if (drv == NULL) {
		printf("Failed to open input device\n");
		return 1;
	}

	GP_EventSetScreenSize(640, 480);

	for (;;) {
		GP_InputDriverLinuxRead(drv);
		
		while (GP_EventQueued()) {
			GP_Event ev;

			GP_EventGet(&ev);
			GP_EventDump(&ev);
		}

		usleep(1000);
	}

	return 0;
}
