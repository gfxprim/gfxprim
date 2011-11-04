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
 * Copyright (C) 2009-2011 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#ifndef CORE_GP_RETCODE_H
#define CORE_GP_RETCODE_H

typedef enum GP_RetCode {
	GP_ESUCCESS,
	GP_EINVAL,
	GP_ENOIMPL,
	GP_EUNPRECISE,
	GP_ENULLPTR,		/* some argument was unexpectedly NULL */
	GP_ENOBACKEND,		/* no backend available */
	GP_EBACKENDLOST,	/* lost connection to the backend */
	GP_EBADCONTEXT,		/* context contains invalid data */
	GP_EBADFILE,		/* error in file, or bad file format */
	GP_ENOENT,		/* no such file or another object */
	GP_ENOMEM,		/* not enough memory */
	GP_EINTR,               /* operation interrupted by user */
	GP_EMAX,
} GP_RetCode;

const char *GP_RetCodeName(GP_RetCode code);

#endif /* CORE_GP_RETCODE_H */
