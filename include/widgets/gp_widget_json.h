//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGET_JSON_H__
#define GP_WIDGET_JSON_H__

#include <widgets/gp_widget.h>

struct json_object;

/**
 * @brief Loads a widget layout given a JSON object.
 *
 * @json A json object.
 * @uids A pointer to a hash table to store widget pointers by UIDs.
 *
 * @return A widget layout or a NULL in case of a failure.
 */
gp_widget *gp_widget_from_json(struct json_object *json, void **uids);

/**
 * @brief Loads a widget layout given a string with JSON layout.
 *
 * @str A string with a JSON layout.
 * @uids A pointer to a hash table to store widget pointers by UIDs.
 *
 * @return A widget layout or a NULL in case of a failure.
 */
gp_widget *gp_widget_from_json_str(const char *str, void **uids);

/**
 * @brief Loads a widget layout given a path to a JSON layout description.
 *
 * @fname A path to a JSON layout file.
 * @uids A pointer to a hash table to store widget pointers by UIDs.
 *
 * @return A widget layout or a NULL in case of a failure.
 */
gp_widget *gp_widget_layout_json(const char *fname, void **uids);

/**
 * @brief Attempts to get a pointer to a function given it's name.
 *
 * This function is used to resolve callbacks from a JSON layout.
 *
 * @fn_name A fucntion name.
 *
 * @return A function pointer.
 */
void *gp_widget_callback_addr(const char *fn_name);

/**
 * @brief Gets a widget pointer given UIDs hash, an id and type.
 *
 * @uids And UIDs hash filled in the JSON parser.
 * @id A widget ID.
 * @type Widget type.
 *
 * @return A widget pointer or NULL if not found.
 */
gp_widget *gp_widget_by_uid(void *uids, const char *uid, enum gp_widget_type type);

/**
 * @brief Gets a widget pointer given UIDs hash, an id and widget_class.
 *
 * @uids And UIDs hash filled in the JSON parser.
 * @id A widget ID.
 * @widget_class Widget class.
 *
 * @return A widget pointer or NULL if not found.
 */
gp_widget *gp_widget_by_cuid(void *uids, const char *uid, enum gp_widget_class widget_class);

#endif /* GP_WIDGET_JSON_H__ */
