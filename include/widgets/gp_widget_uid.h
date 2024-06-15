//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2024 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @file gp_widget_uid.h
 * @brief Widget by unique id lookup.
 *
 * In the JSON layout widgets can have an unique name. All widgets with unique
 * name are put into a hash table when JSON layout is loaded and can be later
 * looked up by these functions once layout has been loaded.
 */
#ifndef GP_WIDGET_UID_H
#define GP_WIDGET_UID_H

#include <utils/gp_htable.h>
#include <widgets/gp_widget.h>

typedef struct gp_widget_uid_map {
	const char *uid;
	enum gp_widget_type type;
	enum gp_widget_class cls;
	size_t offset;
} gp_widget_uid_map;

#define GP_WIDGET_UID(wuid, wtype, structure, member) \
        {.uid = wuid, .type = wtype, .offset = offsetof(structure, member)}

#define GP_WIDGET_CUID(wuid, wclass, structure, member) \
        {.uid = wuid, .cls = wclass, .offset = offsetof(structure, member)}

void gp_widgets_by_uids(gp_htable *uids, gp_widget_uid_map *uid_map, void *structure);

/**
 * @brief Gets a widget pointer given UIDs hash, an id and type.
 *
 * @param uids And UIDs hash filled in the JSON parser.
 * @param uid A widget unique id.
 * @param type Widget type.
 *
 * @return A widget pointer or NULL if not found.
 */
gp_widget *gp_widget_by_uid(gp_htable *uids, const char *uid, enum gp_widget_type type);

/**
 * @brief Gets a widget pointer given UIDs hash, an id and widget_class.
 *
 * @param uids And UIDs hash filled in the JSON parser.
 * @param uid A widget unique id.
 * @param widget_class Widget class.
 *
 * @return A widget pointer or NULL if not found.
 */
gp_widget *gp_widget_by_cuid(gp_htable *uids, const char *uid, enum gp_widget_class widget_class);

#endif /* GP_WIDGET_UID_H */
