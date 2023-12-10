//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2023 Cyril Hrubis <metan@ucw.cz>

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

/**
 * @brief Returns application name.
 *
 * @return Application name.
 */
const char *gp_app_info_name(void);

/**
 * @brief Shows application info in an dialog.
 */
void gp_app_info_dialog_run(void);

/**
 * @brief Prints application info into stdout.
 */
void gp_app_info_print(void);

#endif /* GP_APP_INFO_H */
