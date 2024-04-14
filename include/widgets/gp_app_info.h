//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2023 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @file gp_app_info.h
 * @brief Application info, license, authors, etc.
 */

#ifndef GP_APP_INFO_H
#define GP_APP_INFO_H

#include <widgets/gp_widget.h>

/**
 * @brief Description of the app author.
 */
typedef struct gp_app_info_author {
	/** Author name */
	const char *name;
	/** Author email */
	const char *email;
	/** Copyright years */
	const char *years;
} gp_app_info_author;

/**
 * @brief An application information.
 *
 * Fill in this structure to get the about dialog in the application. The
 * structure has to be called app_info so that it's picked up by the linker
 * automatically.
 *
 * @code
 * struct gp_app_info app_info = {
 *	.name = "app_name",
 *	.desc = "This is a short app description",
 *	.version = "3.14",
 *	.license = "GPL-2.0-or-later",
 *      .url = "http://foo.url",
 *      .authors = (gp_app_info_author []) {
 *              {.name = "Jonh Hacker", .email = "jonh.hacker@foo.url", .years = "2023"},
 *              {}
 *      }
 * };
 * @endcode
 */
typedef struct gp_app_info {
	/** Application name */
	const char *name;
	/** Short application description */
	const char *desc;
	/** Application version */
	const char *version;
	/** Application license */
	const char *license;
	/** URL to application website */
	const char *url;
	/** An {} terminated array of authors */
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
 *
 * This is called automatically by the widget library to show the application
 * info dialog.
 */
void gp_app_info_dialog_run(void);

/**
 * @brief Prints application info into stdout.
 *
 * This is called automatically by the widget library when -i is passed on the
 * commandline.
 */
void gp_app_info_print(void);

#endif /* GP_APP_INFO_H */
