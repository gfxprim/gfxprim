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
#include <errno.h>

#include "params.h"

static const char *param_type_names[] = {
	"bool",
	"integer",
	"float",
	"string",
	"enum",
};

const char *param_type_name(enum param_type type)
{
	if (type > PARAM_ENUM)
		return NULL;

	return param_type_names[type];
}

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
	long l;
	char *end;

	errno = 0;
	l = strtol(val, &end, 0);

	if (*end != '\0')
		return 1;

	if (errno != 0)
		return 1;

	*res = l;

	return 0;
}

int set_float(float *res, char *val)
{
	char *end;
	float f;

	errno = 0;
	f = strtof(val, &end);

	if (*end != '\0')
		return 1;

	if (errno != 0)
		return 1;

	*res = f;

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

#define CALL_ERR_CALLBACK(error, p, value, private) do { \
	if (error != NULL)                               \
		error(p, value, private);                \
} while (0)

int param_parse(const char *params, const struct param *param_desc, void *priv,
                int (*err)(const struct param *self, const char *val, void *priv), ...)
{
	char *par;
	unsigned int n, i;
	va_list va;
	int ret;

	if (params == NULL || *params == '\0')
		return 0;
	
	par = strdup(params);

	if (par == NULL) {
		fprintf(stderr, "Malloc failed :(\n");
		return 1;
	}

	n = count_params(params);

	char *names[n];
	char *values[n];

	split_params(par, names);
	split_values(names, values, n);
	
	va_start(va, err);

	for (i = 0; param_desc[i].name != NULL; i++) {
		void *arg = va_arg(va, void*);
		int pos = 0;

		while ((pos = param_pos(names, param_desc[i].name, pos, n)) >= 0) {
			
			if (values[pos] == NULL || *values[pos] == '\0') {
				CALL_ERR_CALLBACK(err, &param_desc[i], "", priv);
				goto err;
			}
			
			switch (param_desc[i].type) {
			case PARAM_BOOL:
				
			break;
			case PARAM_INT:
				if ((ret = set_int(arg, values[pos]))) {
					CALL_ERR_CALLBACK(err, &param_desc[i],
					                  values[pos], priv);
					goto err;
				}
			break;
			case PARAM_FLOAT:
				if ((ret = set_float(arg, values[pos]))) {
					CALL_ERR_CALLBACK(err, &param_desc[i],
					                  values[pos], priv);
					goto err;
				}
			break;
			case PARAM_STR:
				
			break;
			case PARAM_ENUM:
				if ((ret = set_enum(arg, values[pos],
				     param_desc[i].enum_table))) {
					CALL_ERR_CALLBACK(err, &param_desc[i],
					                  values[pos], priv);
					goto err;
				}
			break;
			}

			if (param_desc[i].check != NULL)
				if ((ret = param_desc[i].check(&param_desc[i], arg))) {
					CALL_ERR_CALLBACK(err, &param_desc[i],
					                  values[pos], priv);
					goto err;
				}
			pos++;
		}
	}

	ret = 0;
err:
	va_end(va);
	free(par);
	return ret;
}
