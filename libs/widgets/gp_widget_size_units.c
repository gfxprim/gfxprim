//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2021 Cyril Hrubis <metan@ucw.cz>

 */

#include <ctype.h>
#include <string.h>

#include <widgets/gp_widget_size_units.h>

static unsigned int get_int(const char **str)
{
	int ret = 0;

	while (**str) {
		switch (**str) {
		case '0' ... '9':
			ret *= 10;
			ret += **str - '0';
		break;
		default:
			return ret;
		}
		(*str)++;
	}

	return ret;
}

enum unit {
	INVALID,
	PX,
	PAD,
	ASC,
};

static int is_pad_px(const char **str)
{
	(*str)++;

	switch (**str) {
	case 'x':
		(*str)++;
		return PX;
	case 'a':
		(*str)++;

		if (**str == 'd') {
			(*str)++;
			return PAD;
		}

		return INVALID;
	break;
	default:
		return INVALID;
	}
}

static int is_asc(const char **str)
{
	(*str)++;

	if (**str != 's')
		return INVALID;

	(*str)++;

	if (**str != 'c')
		return INVALID;

	(*str)++;

	return ASC;
}

static int get_unit(const char **str)
{
	switch (**str) {
	case '0' ... '9':
		return PX;
	case 'p':
		return is_pad_px(str);
	break;
	case 'a':
		return is_asc(str);
	break;
	default:
		return INVALID;
	}
}

int gp_widget_size_units_parse(const char *size, gp_widget_size *ret)
{
	unsigned int num;
	int unit;
	const char *str = size;

	memset(ret, 0, sizeof(*ret));

	while (isspace(*str))
		str++;

	while (*str) {
		num = get_int(&str);

		while (isspace(*str))
			str++;

		if (!*str)
			unit = PX;
		else
			unit = get_unit(&str);

		switch (unit) {
		case PX:
			if (num > UINT16_MAX) {
				GP_WARN("Size string '%s' overflow %u!", size, num);
				return 1;
			}

			if (ret->px) {
				GP_WARN("Duplicit px size in '%s'!", size);
				return 1;
			}

			ret->px = num;
		break;
		case PAD:
			if (num > UINT8_MAX) {
				GP_WARN("Size string '%s' overflow %u!", size, num);
				return 1;
			}

			if (ret->pad) {
				GP_WARN("Duplicit pad size in '%s'!", size);
				return 1;
			}

			ret->pad = num;
		break;
		case ASC:
			if (num > UINT8_MAX) {
				GP_WARN("Size string '%s' overflow %u!", size, num);
				return 1;
			}

			if (ret->asc) {
				GP_WARN("Duplicit asc size in '%s'!", size);
				return 1;
			}

			ret->asc = num;
		break;
		case INVALID:
			GP_WARN("Invalid unit in '%s'!", size);
			return 1;
		}

		while (isspace(*str))
			str++;
	}

	return 0;
}
