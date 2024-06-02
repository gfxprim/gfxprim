//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2024 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @file gp_app_layout.h
 * @brief Application layout.
 *
 * An app widget layout can be loaded from a JSON description. The application
 * can resolve widgets by unique id see gp_widget_by_uid() and
 * gp_widget_by_cuid() and event handlers are resolved from the application
 * binary at runtime.
 *
 * Example JSON layout
 * --------------------
 * @code{.json}
 * {
 *  "info": {"version": 1, "license": "GPL-2.0-or-later", "author": "Joe Programmer"},
 *  "layout": {
 *   "widgets": [
 *    {"type": "button", "label": "Button", "on_event": "button_event"}
 *   ]
 *  }
 * }
 * @endcode
 *
 * A JSON layout always starts with an info block. The required parameters in
 * the info block are license and version number. The license should be a SPDX
 * licence identifier. Currently the version is set to 1 and the number will be
 * increased if there are incompatible changes done in the format in a
 * subsequent releases.
 *
 * The info block is followed by a layout block, which describes the widgets.
 *
 * Widget `type` defaults to `grid` and `cols` and `rows` default to 1 for a
 * grid widget so the outer widget is a grid with exactly one cell.
 *
 * The inner widget `type` is a gp_widget_button.h and as such it should have
 * either `btype` or `label`, in this case we have a button with a `label`.
 *
 * The button gp_widget::on_event() is set to `button_event` which means that,
 * at runtime, fuction with this name will be resolved by the dynamic linker
 * and called when the button has been pressed.
 *
 * Generic widget JSON attributes
 * ------------------------------
 *
 * |   Attribute  |  Type  | Default  | Description
 * |--------------|--------|----------|----------------------------------------------
 * |   **uid**    | string |          | Widget universal id. Must be unique.
 * | **disabled** |  bool  |  false   | Disables (grays out) widget and all its children, see gp_widget_disable.h.
 * |   **type**   | string |   grid   | A #gp_widget_type, e.g. "button".
 * |   **align**  | string | `center` | Sets #gp_widget::align both **haling** and **valign** to `center` or `fill`.
 * |  **haling**  | string | `center` | Horizontal alignment one of `center`, `left`, `right`, `fill`.
 * |  **valing**  | string | `center` | Vertical alignment one of `center`, `top`, `bottom`, `fill`.
 * | **on_event** | string |          | A gp_widget::on_event() handler function name.
 */

#ifndef GP_APP_LAYOUT_H
#define GP_APP_LAYOUT_H

#include <widgets/gp_widget_types.h>

/**
 * @brief Loads a widget layout given an application name.
 *
 * This is a wrapper for gp_app_layout_load2() with NULL passed as `callbacks`.
 *
 * @param app_name An application name, usually the same as the binary name.
 * @param uids A pointer to store the hash table of mappings between widget
 *        unique ids and pointers to.
 * @return An application widget layout or NULL if no layout has been found or
 *         if it couldn't be parsed.
 */
gp_widget *gp_app_layout_load(const char *app_name, gp_htable **uids);

/**
 * @brief Loads a widget layout given an application name.
 *
 * Attempts to load an application layout based on `app_name`. The library
 * first looks for a layout into a user directory and if there is no layout
 * found there the system directory is used instead. This means that user can
 * override system application layout by creating one in the user home
 * directory.
 *
 * The function looks for the layout in `$HOME/.config/` and `/etc/`.
 *
 * Widget gp_widget::on_event() function addresses and structure pointers e.g.
 * #gp_widget_table_ops are first attempted to be resolved from the `callbacks`
 * array. If `callbacks` was passed as NULL or if particular callback wasn't
 * found in the array the library will attempt to resolve the pointers from the
 * runing application by `dlsym()`.
 *
 * @param app_name An application name, usually the same as the binary name.
 * @param uids A pointer to store the hash table of mappings between widget
 *        unique ids and pointers to.
 * @return An application widget layout or NULL if no layout has been found or
 *         if it couldn't be parsed.
 */
gp_widget *gp_app_layout_load2(const char *app_name,
                               const gp_widget_json_callbacks *const callbacks,
                               gp_htable **uids);

/**
 * @brief Loads an application fragment given application name and fragment name.
 *
 * The function looks for the layout in `$HOME/.config/` and `/etc/`.
 *
 * @param app_name An application name, usually the same as the binary name.
 * @param layout_name A layout name.
 * @param callbacks Optional callbacks description, NULL if not used.
 * @param uids An pointer to store the hash table of UIDs to.
 *
 * @return An application widget layout or NULL in a case of failure.
 */
gp_widget *gp_app_layout_fragment_load(const char *app_name, const char *layout_name,
                                       const gp_widget_json_callbacks *const callbacks,
                                       gp_htable **uids);

#endif /* GP_APP_LAYOUT_H */
