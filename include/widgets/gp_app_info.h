//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2024 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @file gp_app_info.h
 * @brief Application info, license, authors, etc.
 *
 * Application info is a structure, that if defined, stores an application
 * information such as name, licence, description, version and a list of authors.
 *
 * The application info is shown as an dialog when 'Ctrl+i' is pressed and
 * printed into the stdout when application is passed '-i' on the command line.
 *
 * @attention The app info structure has to be populated in order to have the
 *            application name in the window tittle.
 */

#ifndef GP_APP_INFO_H
#define GP_APP_INFO_H

#include <widgets/gp_widget.h>

/**
 * @brief Description of the app author.
 */
typedef struct gp_app_info_author {
	/** @brief Author name */
	const char *name;
	/** @brief Author email */
	const char *email;
	/** @brief Copyright years */
	const char *years;
} gp_app_info_author;

/**
 * @brief An application information.
 *
 * Fill in this structure to get the about dialog in the application.
 *
 * @attention The structure has to be called app_info so that it's picked up by
 *            the linker automatically.
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
 *
 * @image html widgets_app_info.png
 */
typedef struct gp_app_info {
	/** @brief Application name */
	const char *name;
	/** @brief Short application description */
	const char *desc;
	/** @brief Application version */
	const char *version;
	/** @brief Application license */
	const char *license;
	/** @brief URL to application website */
	const char *url;
	/** @brief An {} terminated array of authors */
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
