//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#include <errno.h>
#include <core/gp_debug.h>
#include <utils/gp_vec_str.h>
#include <widgets/gp_widget_json.h>
#include <widgets/gp_widget_app.h>

static gp_widget *try_load_layout(const char *pathname, void **uids)
{
	if (access(pathname, R_OK)) {
		GP_DEBUG(3, "File '%s' does not exists or is not readable", pathname);
		return NULL;
	}

	GP_DEBUG(3, "Trying '%s'", pathname);

	return gp_widget_layout_json(pathname, uids);
}

gp_widget *gp_app_layout_load(const char *app_name, void **uids)
{
	gp_widget *layout;
	char *pathname;
	const char *home = getenv("HOME");

	pathname = gp_vec_printf(NULL, "./%s.json", app_name);
	if (!pathname)
		return NULL;

	layout = try_load_layout(pathname, uids);
	if (layout)
		goto ret;

	if (home) {
		pathname = gp_vec_printf(pathname, "%s/.config/gp_apps/%s/layout.json", home, app_name);
		if (!pathname)
			return NULL;

		layout = try_load_layout(pathname, uids);
		if (layout)
			goto ret;
	}

	pathname = gp_vec_printf(pathname, "/etc/gp_apps/%s/layout.json", app_name);
	if (!pathname)
		return NULL;

	layout = try_load_layout(pathname, uids);
	if (layout)
		goto ret;

	GP_WARN("No layout for application '%s' found!", app_name);

ret:
	gp_vec_free(pathname);
	return layout;
}
