//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2022 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGET_JSON_H
#define GP_WIDGET_JSON_H

#include <utils/gp_json.h>
#include <utils/gp_htable.h>
#include <widgets/gp_widget.h>

extern const gp_json_obj *gp_widget_json_attrs;

/**
 * A structure to pass on_event/table/etc callbacks.
 *
 * @on_event A callback pointer.
 * @id A callback name.
 */
typedef struct gp_widget_json_addr {
	union {
		void *addr;
		int (*on_event)(gp_widget_event *);
		const struct gp_widget_table_col_ops *table_col_ops;
	};
	const char *id;
} gp_widget_json_addr;

/**
 * Structure to pass callbacks to the JSON loader.
 *
 * @default_priv a default priv pointer passed to widgets when allocated.
 * @addrs A NULL terminated array of callbacks, sorted alphabetically by ids.
 */
struct gp_widget_json_callbacks {
	void *default_priv;
	const gp_widget_json_addr *addrs;
};

/**
 * Context to propagate values top down and bottom up.
 *
 * @uids A hash table to store the id -> widget mapping to. Passed down from
 *       the application.
 * @focused A widget that should be focused when layout is loaded. Passed up
 *          from the json loader.
 * @callbacks An optional table of widget callbacks.
 */
struct gp_widget_json_ctx {
	gp_htable **uids;
	gp_widget *focused;
	const gp_widget_json_callbacks *callbacks;
};

/**
 * @brief Loads a widget layout given a JSON object.
 *
 * @json A json object.
 * @uids A pointer to a hash table to store widget pointers by UIDs.
 * @ctx A widget JSON loader context, usually passed from our caller.
 *
 * @return A widget layout or a NULL in case of a failure.
 */
gp_widget *gp_widget_from_json(gp_json_reader *json, gp_json_val *val, gp_widget_json_ctx *ctx);

/**
 * @brief Loads a widget layout given a string with JSON layout.
 *
 * @str A string with a JSON layout.
 * @callbacks An optional NULL terminated array of application callbacks.
 * @uids A pointer to a hash table to store widget pointers by UIDs.
 *
 * @return A widget layout or a NULL in case of a failure.
 */
gp_widget *gp_widget_from_json_str(const char *str,
                                   const gp_widget_json_callbacks *const callbacks,
                                   gp_htable **uids);

/**
 * @brief Loads a widget layout given a path to a JSON layout description.
 *
 * @fname A path to a JSON layout file.
 * @callbacks An optional NULL terminated array of application callbacks.
 * @uids A pointer to a hash table to store widget pointers by UIDs.
 *
 * @return A widget layout or a NULL in case of a failure.
 */
gp_widget *gp_widget_layout_json(const char *fname,
                                 const gp_widget_json_callbacks *const callbacks,
                                 gp_htable **uids);

/**
 * @brief Attempts to get a pointer to a function given it's name.
 *
 * This function is used to resolve callbacks from a JSON layout.
 *
 * @fn_name A fucntion name.
 * @ctx A widget JSON loader context, usually passed from our caller.
 *
 * @return A function pointer.
 */
void *gp_widget_callback_addr(const char *fn_name,
                              const gp_widget_json_ctx *ctx);

/**
 * @brief Attempts to get a pointer to a structure given it's name.
 *
 * This function is used to resolve structures from a JSON layout.
 *
 * @struct_name A structure name.
 * @ctx A widget JSON loader context, usually passed from our caller.
 *
 * @return A structure pointer.
 */
void *gp_widget_struct_addr(const char *struct_name,
                            const gp_widget_json_ctx *ctx);

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

#endif /* GP_WIDGET_JSON_H */
