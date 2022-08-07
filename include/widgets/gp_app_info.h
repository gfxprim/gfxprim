//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2022 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_APP_INFO_H
#define GP_APP_INFO_H

#include <widgets/gp_widget.h>

typedef struct gp_app_info_author {
	const char *name;
	const char *email;
	const char *years;
} gp_app_info_author;

typedef struct gp_app_info {
	const char *name;
	const char *desc;
	const char *version;
	const char *license;
	const char *url;
	gp_app_info_author *authors;
} gp_app_info;

void gp_app_info_set(gp_app_info *app_info);

void gp_app_info_dialog_run(void);

void gp_app_info_print(void);

#endif /* GP_APP_INFO_H */
