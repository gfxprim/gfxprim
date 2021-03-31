//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#include <time.h>
#include <widgets/gp_widgets.h>

static gp_widget *wday;
static gp_widget *wmon;
static gp_widget *wyear;
static gp_widget *whour;
static gp_widget *wmin;
static gp_widget *wsec;

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

	days = days_in_mon(gp_widget_int_get(wmon), gp_widget_int_get(wyear));

	gp_widget_int_set_max(wday, days);

	return 0;
}

int load_sys_time(gp_widget_event *ev)
{
	struct tm tm;
	time_t t;

	(void)ev;

	time(&t);
	localtime_r(&t, &tm);

	if (wsec)
		gp_widget_int_set(wsec, tm.tm_sec);

	if (wmin)
		gp_widget_int_set(wmin, tm.tm_min);

	if (whour)
		gp_widget_int_set(whour, tm.tm_hour);

	if (wday)
		gp_widget_int_set(wday, tm.tm_mday);

	if (wmon)
		gp_widget_int_set(wmon, tm.tm_mon + 1);

	if (wyear)
		gp_widget_int_set(wyear, tm.tm_year + 1900);

	return 0;
}

int main(int argc, char *argv[])
{
	static gp_htable *uids;

	gp_widget *layout = gp_widget_layout_json("datetime.json", &uids);
	if (!layout)
		return 0;

	wday  = gp_widget_by_uid(uids, "day", GP_WIDGET_SPINNER);
	wmon  = gp_widget_by_uid(uids, "mon", GP_WIDGET_SPINNER);
	wyear = gp_widget_by_uid(uids, "year", GP_WIDGET_SPINNER);
	whour = gp_widget_by_uid(uids, "hour", GP_WIDGET_SPINNER);
	wmin  = gp_widget_by_uid(uids, "min", GP_WIDGET_SPINNER);
	wsec  = gp_widget_by_uid(uids, "sec", GP_WIDGET_SPINNER);

	gp_widgets_main_loop(layout, "Date & Time", NULL, argc, argv);

	return 0;
}
