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

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdarg.h>

#include "params.h"

static unsigned int count_params(const char *params)
{
	unsigned int ret = 1, i;
	char prev = ':';

	for (i = 0; params[i] != '\0'; i++) {
		
		if (params[i] == ':' && prev != ':')
			ret++;

		prev = params[i];
	}

	return ret;
}

static void split_params(char *params, char **names)
{
	unsigned int i, n = 0;
	char prev = ':';

	for (i = 0; params[i] != '\0'; i++) {
		
		if (params[i] != ':' && prev == ':')
			names[n++] = &params[i];

		prev = params[i];

		if (params[i] == ':')
			params[i] = '\0';
	}
}

static void do_split(char *param, char **value)
{
	unsigned int i;

	*value = NULL;

	for (i = 0; param[i] != '\0'; i++) {
		
		if (param[i] == '=' || isspace(param[i])) {
			param[i] = '\0';
			*value = &param[i+1];
		}
	}
}

static void split_values(char **names, char **values, unsigned int n)
{
	unsigned int i;

	for (i = 0; i < n; i++)
		do_split(names[i], &values[i]);
}

int param_pos(char *names[], const char *name, unsigned int start, unsigned int n)
{
	unsigned int i;

	for (i = start; i < n; i++)
		if (!strcasecmp(names[i], name))
			return i;

	return -1;
}

int set_int(int *res, char *val)
{
	//TODO: error checks
	*res = atoi(val);

	return 0;
}

int set_float(float *res, char *val)
{
	//TODO: error checks
	*res = atof(val);

	return 0;
}

int set_enum(int *res, char *val, const char *enums[])
{
	unsigned int i;

	for (i = 0; enums[i] != NULL; i++)
		if (!strcasecmp(enums[i], val)) {
			*res = i;
			return 0;
		}

	return 1;
}

int param_parse(const char *params, const struct param *param_desc, ...)
{
	char *par = strdup(params);
	unsigned int n, i;
	va_list va;

	if (par == NULL) {
		fprintf(stderr, "Malloc failed :(\n");
		return 1;
	}

	n = count_params(params);

	char *names[n];
	char *values[n];

	split_params(par, names);
	split_values(names, values, n);
	
	va_start(va, param_desc);

	for (i = 0; param_desc[i].name != NULL; i++) {
		void *arg = va_arg(va, void*);
		int pos = 0, ret;

		while ((pos = param_pos(names, param_desc[i].name, pos, n)) >= 0) {
			switch (param_desc[i].type) {
			case PARAM_BOOL:
				
			break;
			case PARAM_INT:
				if ((ret = set_int(arg, values[pos])))
					return ret;
			break;
			case PARAM_FLOAT:
				if ((ret = set_float(arg, values[pos])))
					return ret;
			break;
			case PARAM_STR:
				
			break;
			case PARAM_ENUM:
				if ((ret = set_enum(arg, values[pos],
				     param_desc[i].enum_table)))
					return ret;
			break;
			}

			if (param_desc[i].check != NULL)
				param_desc[i].check(&param_desc[i], arg);
			pos++;
		}
	}

	va_end(va);
	free(par);

	return 0;
}
