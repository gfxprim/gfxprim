//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2022 Cyril Hrubis <metan@ucw.cz>

 */

#include <errno.h>
#include <core/gp_debug.h>
#include <utils/gp_vec_str.h>
#include <widgets/gp_widget_json.h>
#include <widgets/gp_widget_app.h>

static gp_widget *try_load_layout(const char *pathname, gp_htable **uids)
{
	if (access(pathname, R_OK)) {
		GP_DEBUG(3, "File '%s' does not exists or is not readable", pathname);
		return NULL;
	}

	GP_DEBUG(3, "Trying '%s'", pathname);

	return gp_widget_layout_json(pathname, NULL, uids);
}

static gp_widget *layout_load(const char *app_name, const char *json_name, gp_htable **uids)
{
	gp_widget *layout;
	char *pathname;
	const char *home = getenv("HOME");

	pathname = gp_vec_printf(NULL, "./%s.json", json_name);
	if (!pathname)
		return NULL;

	layout = try_load_layout(pathname, uids);
	if (layout)
		goto ret;

	if (home) {
		pathname = gp_vec_printf(pathname,
		                         "%s/.config/gp_apps/%s/%s.json",
		                         home, app_name, json_name);

		if (!pathname)
			return NULL;

		layout = try_load_layout(pathname, uids);
		if (layout)
			goto ret;
	}

	pathname = gp_vec_printf(pathname, "/etc/gp_apps/%s/%s.json",
	                         app_name, json_name);

	if (!pathname)
		return NULL;

	layout = try_load_layout(pathname, uids);
	if (layout)
		goto ret;

	GP_WARN("No layout '%s.json' for application '%s' found!",
	        json_name, app_name);

ret:
	gp_vec_free(pathname);
	return layout;
}

gp_widget *gp_app_layout_load(const char *app_name, gp_htable **uids)
{
	return layout_load(app_name, "layout", uids);
}

gp_widget *gp_app_layout_fragment_load(const char *app_name,
                                       const char *fragment_name, void *uids)
{
	return layout_load(app_name, fragment_name, uids);
}

static struct gp_app app = {
	.ev_mask = GP_APP_DEFAULT_EVENT_MASK,
};

const struct gp_app *gp_app = &app;

void gp_app_event_mask(enum gp_widget_event_type ev_type)
{
	app.ev_mask &= ~(1<<ev_type);
}

void gp_app_event_unmask(enum gp_widget_event_type ev_type)
{
	app.ev_mask |= (1<<ev_type);
}

void gp_app_on_event_set(int (*on_event)(gp_widget_event *ev))
{
	app.on_event = on_event;
}
