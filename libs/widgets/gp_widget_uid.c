//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2022 Cyril Hrubis <metan@ucw.cz>

 */

#include <core/gp_debug.h>
#include <utils/gp_htable.h>
#include <widgets/gp_widget_ops.h>
#include <widgets/gp_widget_uid.h>

void gp_widgets_by_uids(gp_htable *uids, gp_widget_uid_map *uid_map, void *structure)
{
	unsigned int i;

	for (i = 0; uid_map[i].uid; i++) {
		gp_widget *res;

		if (uid_map[i].cls)
			res = gp_widget_by_cuid(uids, uid_map[i].uid, uid_map[i].cls);
		else
			res = gp_widget_by_uid(uids, uid_map[i].uid, uid_map[i].type);

		if (res)
			*(gp_widget**)((char*)structure + uid_map[i].offset) = res;
	}
}

gp_widget *gp_widget_by_uid(gp_htable *uids, const char *uid, enum gp_widget_type type)
{
	gp_widget *ret = gp_htable_get(uids, uid);

	if (!ret)
		return NULL;

	if (ret->type != type) {
		GP_WARN("Unexpected widget (uid='%s') type %s expected %s",
		        uid, gp_widget_type_name(ret->type),
		        gp_widget_type_name(type));
		return NULL;
	}

	return ret;
}

gp_widget *gp_widget_by_cuid(gp_htable *uids, const char *uid, enum gp_widget_class widget_class)
{
	gp_widget *ret = gp_htable_get(uids, uid);

	if (!ret)
		return NULL;

	if (ret->widget_class != widget_class) {
		GP_WARN("Unexpected widget (uid='%s') class %s expected %s",
		        uid, gp_widget_class_name(ret->widget_class),
		        gp_widget_class_name(widget_class));
		return NULL;
	}

	return ret;
}
