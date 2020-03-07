// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2011 Cyril Hrubis <metan@ucw.cz>
 */

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
	const char *desc;
	const char **enum_table;

	/* called after parameter is set */
	int (*check)(const struct param *self, void *val, int count);
};

const char *param_type_name(enum param_type type);

void param_describe(const struct param *param_desc, const char *prefix);

int param_parse(const char *params, const struct param *param_desc, void *priv,
               int (*err)(const struct param *self, const char *val, void *priv), ...);

#endif /* PARAMS_H */
