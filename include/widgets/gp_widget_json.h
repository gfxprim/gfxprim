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
 * @brief A structure to pass widget callbacks.
 */
typedef struct gp_widget_json_addr {
	union {
		/** @brief A structure address. */
		void *addr;
		/** @brief A widget event handler. */
		int (*on_event)(gp_widget_event *);
		/** @brief A table widget column ops. */
		const struct gp_widget_table_col_ops *table_col_ops;
	};
	/** @brief A callback name used to resolve the target widget on runtime. */
	const char *id;
} gp_widget_json_addr;

/**
 * @brief Structure to pass callbacks to the JSON loader.
 */
struct gp_widget_json_callbacks {
	/** @brief A default priv pointer passed to widgets when allocated. */
	void *default_priv;
	/**
	 * @brief A NULL terminated array of callbacks.
	 *
	 * @attention: The array __must__ be sorted by addr::id!
	 */
	const gp_widget_json_addr *addrs;
};

/**
 * @brief A context to propagate values top down and bottom up.
 */
struct gp_widget_json_ctx {
	/**
	 * @brief A hash table to store the id -> widget mapping to.
	 *
	 * Passed down from the application.
	 */
	gp_htable **uids;
	/**
	 * @brief A widget that should be focused when layout is loaded.
	 *
	 * Passed down from the json loader.
	 */
	gp_widget *focused;
	/**
	 * @brief An optional table of widget callbacks.
	 *
	 * If not set the callbacks are resolved by the dynamic linker.
	 */
	const gp_widget_json_callbacks *callbacks;
};

/**
 * @brief Loads a widget layout given a JSON object.
 *
 * @param json A json object.
 * @param uids A pointer to a hash table to store widget pointers by UIDs.
 * @param ctx A widget JSON loader context, usually passed from our caller.
 *
 * @return A widget layout or a NULL in case of a failure.
 */
gp_widget *gp_widget_from_json(gp_json_reader *json, gp_json_val *val, gp_widget_json_ctx *ctx);

/**
 * @brief Loads a widget layout given a string with JSON layout.
 *
 * @param str A string with a JSON layout.
 * @param callbacks An optional NULL terminated array of application callbacks.
 * @param uids A pointer to a hash table to store widget pointers by UIDs.
 *
 * @return A widget layout or a NULL in case of a failure.
 */
gp_widget *gp_widget_from_json_str(const char *str,
                                   const gp_widget_json_callbacks *const callbacks,
                                   gp_htable **uids);

/**
 * @brief Loads a widget layout given a path to a JSON layout description.
 *
 * @param fname A path to a JSON layout file.
 * @param callbacks An optional NULL terminated array of application callbacks.
 * @param uids A pointer to a hash table to store widget pointers by UIDs.
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
 * @param fn_name A fucntion name.
 * @param ctx A widget JSON loader context, usually passed from our caller.
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
 * @param struct_name A structure name.
 * @param ctx A widget JSON loader context, usually passed from our caller.
 *
 * @return A structure pointer.
 */
void *gp_widget_struct_addr(const char *struct_name,
                            const gp_widget_json_ctx *ctx);

#endif /* GP_WIDGET_JSON_H */
