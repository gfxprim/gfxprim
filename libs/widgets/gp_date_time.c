//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#include <stdlib.h>
#include <stdio.h>

#include <gp_date_time.h>

static const char *const months[] = {
	"Jan",
	"Feb",
	"Mar",
	"Apr",
	"May",
	"Jun",
	"Jul",
	"Aug",
	"Sep",
	"Oct",
	"Nov",
	"Dec",
};

const char *gp_str_time_diff(char *buf, size_t buf_len, time_t time, time_t now)
{
	time_t diff = now - time;
	int mon, year;
	struct tm *t;

	if (diff < 0)
		return "Future!?";

	if (diff < 30)
		return "Now";

	if (diff < 90)
		return "Minute";

	if (diff < 60 * 60) {
		snprintf(buf, buf_len, "%li Minutes", (diff+30)/60);
		return buf;
	}

	if (diff < 60 * 90)
		return "Hour";

	if (diff < 60 * 60 * 24) {
		snprintf(buf, buf_len, "%li Hours", (diff+1800)/3600);
		return buf;
	}

	if (diff < 60 * 60 * 24)
		return "Day";

	if (diff < 60 * 60 * 24 * 30) {
		snprintf(buf, buf_len, "%li Days", (diff + 3600 * 12)/(3600 * 24));
		return buf;
	}

	t = localtime(&time);
	mon = t->tm_mon;
	year = t->tm_year;

	t = localtime(&now);

	if (year != t->tm_year)
		snprintf(buf, buf_len, "%i", year + 1900);
	else if (mon != t->tm_mon)
		return months[mon];

	return buf;
}
