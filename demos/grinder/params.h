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

#ifndef PARAMS_H
#define PARAMS_H

enum param_type {
	PARAM_BOOL,
	PARAM_INT,
	PARAM_FLOAT,
	PARAM_STR,
	PARAM_ENUM,
};

struct param {
	const char *name;
	enum param_type type;
	const char **enum_table;

	int (*check)(const struct param *self, void *val);
};

const char *param_type_name(enum param_type type);

int param_parse(const char *params, const struct param *param_desc, void *priv, 
               int (*err)(const struct param *self, const char *val, void *priv), ...);

#endif /* PARAMS_H */
