//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2021 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>
#include <json-c/json.h>
#include <gp_widgets.h>
#include <gp_string.h>

gp_widget *gp_widget_choice_from_json(unsigned int widget_type,
                                      json_object *json, gp_htable **uids)
{
	json_object *labels = NULL;
	int sel_label = 0;

	(void)uids;

	json_object_object_foreach(json, key, val) {
		if (!strcmp(key, "choices"))
			labels = val;
		else if (!strcmp(key, "selected"))
			sel_label = json_object_get_int(val);
		else
			GP_WARN("Invalid radiobutton key '%s'", key);
	}

	if (!labels) {
		GP_WARN("Missing labels array!");
		return NULL;
	}

	if (!json_object_is_type(labels, json_type_array)) {
		GP_WARN("Buttons has to be array of strings!");
		return NULL;
	}

	unsigned int i, label_cnt = json_object_array_length(labels);
	const char *labels_arr[label_cnt];

	if (sel_label < 0 || (unsigned int)sel_label >= label_cnt) {
		GP_WARN("Invalid selected button %i", sel_label);
		sel_label = 0;
	}

	for (i = 0; i < label_cnt; i++) {
		json_object *label = json_object_array_get_idx(labels, i);
		labels_arr[i] = json_object_get_string(label);

		if (!labels_arr[i])
			GP_WARN("Button %i must be string!", i);
	}

	return gp_widget_choice_new(widget_type,
	                            labels_arr, label_cnt, sel_label,
	                            NULL, NULL);
}

gp_widget *gp_widget_choice_new(unsigned int widget_type,
                                const char *choices[],
                                unsigned int choice_cnt,
                                unsigned int selected,
				int (*on_event)(gp_widget_event *self),
				void *priv)
{
	size_t size = sizeof(struct gp_widget_choice)
	              + gp_string_arr_size(choices, choice_cnt);

	gp_widget *ret = gp_widget_new(widget_type, GP_WIDGET_CLASS_CHOICE, size);
	if (!ret)
		return NULL;

	ret->choice->sel = selected;
	ret->choice->choices = gp_string_arr_copy(choices, choice_cnt, ret->choice->payload);
	ret->choice->max = choice_cnt;

	gp_widget_event_handler_set(ret, on_event, priv);

	return ret;
}

void gp_widget_choice_set(gp_widget *self, unsigned int sel)
{
	GP_WIDGET_CLASS_ASSERT(self, GP_WIDGET_CLASS_CHOICE, );

	if (self->choice->sel == sel)
		return;

	self->choice->sel = sel;

	gp_widget_redraw(self);
}

unsigned int gp_widget_choice_get(gp_widget *self)
{
	GP_WIDGET_CLASS_ASSERT(self, GP_WIDGET_CLASS_CHOICE, 0);

	return self->choice->sel;
}
