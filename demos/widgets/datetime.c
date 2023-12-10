//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#include <time.h>
#include <widgets/gp_widgets.h>

struct widgets {
	gp_widget *day;
	gp_widget *mon;
	gp_widget *year;
	gp_widget *hour;
	gp_widget *min;
	gp_widget *sec;
};

static struct widgets widgets;

static int is_leap(int year)
{
	if (year % 4 == 0) {
		if (year % 100 == 0) {
			if (year % 400 == 0)
				return 1;

			return 0;
		}

		return 1;
	}

	return 0;
}

static int months[] = {
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

static int days_in_mon(int mon, int year)
{
	if (mon == 2)
		return months[mon-1] + is_leap(year);

	return months[mon];
}

int mon_year_on_event(gp_widget_event *ev)
{
	int days;

	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	days = days_in_mon(gp_widget_int_val_get(widgets.mon), gp_widget_int_val_get(widgets.year));

	gp_widget_int_max_set(widgets.day, days);

	return 0;
}

int load_sys_time(gp_widget_event *ev)
{
	struct tm tm;
	time_t t;

	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	time(&t);
	localtime_r(&t, &tm);

	if (widgets.sec)
		gp_widget_int_val_set(widgets.sec, tm.tm_sec);

	if (widgets.min)
		gp_widget_int_val_set(widgets.min, tm.tm_min);

	if (widgets.hour)
		gp_widget_int_val_set(widgets.hour, tm.tm_hour);

	if (widgets.day)
		gp_widget_int_val_set(widgets.day, tm.tm_mday);

	if (widgets.mon)
		gp_widget_int_val_set(widgets.mon, tm.tm_mon + 1);

	if (widgets.year)
		gp_widget_int_val_set(widgets.year, tm.tm_year + 1900);

	return 0;
}

static gp_widget_uid_map uid_map[] = {
	GP_WIDGET_UID("day", GP_WIDGET_SPINNER, struct widgets, day),
	GP_WIDGET_UID("mon", GP_WIDGET_SPINNER, struct widgets, mon),
	GP_WIDGET_UID("year", GP_WIDGET_SPINNER, struct widgets, year),
	GP_WIDGET_UID("hour", GP_WIDGET_SPINNER, struct widgets, hour),
	GP_WIDGET_UID("min", GP_WIDGET_SPINNER, struct widgets, min),
	GP_WIDGET_UID("sec", GP_WIDGET_SPINNER, struct widgets, sec),
	{}
};

int main(int argc, char *argv[])
{
	static gp_htable *uids;

	gp_widget *layout = gp_widget_layout_json("datetime.json", NULL, &uids);
	if (!layout)
		return 0;

	gp_widgets_by_uids(uids, uid_map, &widgets);
	gp_htable_free(uids);

	gp_widgets_main_loop(layout, NULL, argc, argv);

	return 0;
}
