//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2024 Cyril Hrubis <metan@ucw.cz>

 */

#include <errno.h>
#include <core/gp_debug.h>
#include <utils/gp_vec_str.h>
#include <widgets/gp_widget_json.h>
#include <widgets/gp_app_layout.h>

static gp_widget *try_load_layout(const char *pathname,
                                  const gp_widget_json_callbacks *const callbacks,
                                  gp_htable **uids)
{
	if (access(pathname, R_OK)) {
		GP_DEBUG(3, "File '%s' does not exists or is not readable", pathname);
		return NULL;
	}

	GP_DEBUG(3, "Trying '%s'", pathname);

	return gp_widget_layout_json(pathname, callbacks, uids);
}

static gp_widget *layout_load(const char *app_name, const char *json_name,
                              const gp_widget_json_callbacks *const callbacks,
                              gp_htable **uids)
{
	gp_widget *layout;
	char *pathname;
	const char *home = getenv("HOME");

	pathname = gp_vec_printf(NULL, "./%s.json", json_name);
	if (!pathname)
		return NULL;

	layout = try_load_layout(pathname, callbacks, uids);
	if (layout)
		goto ret;

	if (home) {
		pathname = gp_vec_printf(pathname,
		                         "%s/.config/gp_apps/%s/%s.json",
		                         home, app_name, json_name);

		if (!pathname)
			return NULL;

		layout = try_load_layout(pathname, callbacks, uids);
		if (layout)
			goto ret;
	}

	pathname = gp_vec_printf(pathname, "/etc/gp_apps/%s/%s.json",
	                         app_name, json_name);

	if (!pathname)
		return NULL;

	layout = try_load_layout(pathname, callbacks, uids);
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
	return layout_load(app_name, "layout", NULL, uids);
}

gp_widget *gp_app_layout_load2(const char *app_name,
                               const gp_widget_json_callbacks *const callbacks,
                               gp_htable **uids)
{
	return layout_load(app_name, "layout", callbacks, uids);
}

gp_widget *gp_app_layout_fragment_load(const char *app_name, const char *layout_name,
                                       const gp_widget_json_callbacks *const callbacks,
                                       gp_htable **uids)
{
	return layout_load(app_name, layout_name, callbacks, uids);
}
