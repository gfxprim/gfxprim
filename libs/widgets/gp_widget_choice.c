//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2021 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>
#include <widgets/gp_widgets.h>
#include <widgets/gp_string.h>

enum keys {
	CHOICES,
	SELECTED,
};

static const gp_json_obj_attr attrs[] = {
	GP_JSON_OBJ_ATTR("choices", GP_JSON_ARR),
	GP_JSON_OBJ_ATTR("selected", GP_JSON_INT),
};

static const gp_json_obj obj_filter = {
	.attrs = attrs,
	.attr_cnt = GP_ARRAY_SIZE(attrs),
};

static gp_widget *alloc_choice(unsigned int widget_type,
                               size_t payload_size, unsigned int choice_cnt)
{
	size_t size = sizeof(struct gp_widget_choice) + payload_size;
	gp_widget *ret;

	ret = gp_widget_new(widget_type, GP_WIDGET_CLASS_CHOICE, size);
	if (!ret)
		return NULL;

	ret->choice->max = choice_cnt;
	ret->choice->choices = (void*)ret->choice->payload;
	ret->choice->sel = 0;

	return ret;
}

static gp_widget *parse_choices(unsigned int widget_type,
                                gp_json_buf *json, gp_json_val *val)
{
	gp_json_state state = gp_json_state_start(json);
	gp_widget *ret;
	size_t size = 0;
	unsigned int cnt = 0;

	GP_JSON_ARR_FOREACH(json, val) {
		if (val->type == GP_JSON_STR) {
			size += strlen(val->val_str) + 1 + sizeof(void*);
			cnt++;
		} else {
			gp_json_warn(json, "Invalid choice type!");
		}
	}

	ret = alloc_choice(widget_type, size, cnt);
	if (!ret)
		return NULL;

	gp_json_state_load(json, state);

	char *save = val->buf;

	val->buf = (void*)ret->choice->payload + cnt * sizeof(void*);

	cnt = 0;

	GP_JSON_ARR_FOREACH(json, val) {
		if (val->type != GP_JSON_STR)
			continue;

		ret->choice->choices[cnt++] = val->buf;
		val->buf += strlen(val->buf) + 1;
	}


	val->buf = save;

	return ret;
}

gp_widget *gp_widget_choice_from_json(unsigned int widget_type,
                                      gp_json_buf *json, gp_json_val *val,
                                      gp_widget_json_ctx *ctx)
{
	unsigned int sel = 0;
	gp_widget *ret = NULL;

	(void)ctx;

	GP_JSON_OBJ_FILTER(json, val, &obj_filter, gp_widget_json_attrs) {
		switch (val->idx) {
		case CHOICES:
			ret = parse_choices(widget_type, json, val);
		break;
		case SELECTED:
			if (val->val_int < 0)
				gp_json_warn(json, "Invalid value!");
			else
				sel = val->val_int;
		break;
		}
	}

	if (!ret) {
		gp_json_warn(json, "Missing choices array!");
		return NULL;
	}

	if (sel >= ret->choice->max)
		gp_json_warn(json, "Invalid selected label %i", sel);
	else
		ret->choice->sel = sel;

	return ret;
}

gp_widget *gp_widget_choice_new(unsigned int widget_type,
                                const char *choices[],
                                unsigned int choice_cnt,
                                unsigned int selected,
				int (*on_event)(gp_widget_event *self),
				void *priv)
{
	gp_widget *ret;
	size_t payload_size = gp_string_arr_size(choices, choice_cnt);

	ret = alloc_choice(widget_type, payload_size, choice_cnt);
	if (!ret)
		return NULL;

	gp_string_arr_copy(choices, choice_cnt, ret->choice->payload);

	if (selected >= choice_cnt)
		GP_WARN("Invalid selected choice %u, max=%u", selected, choice_cnt);
	else
		ret->choice->sel = selected;

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
