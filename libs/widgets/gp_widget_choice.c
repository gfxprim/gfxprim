//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2023 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>
#include <widgets/gp_widgets.h>
#include <widgets/gp_string.h>
#include "gp_widget_choice_priv.h"

static const char *get_choice(gp_widget *self, size_t idx)
{
	struct gp_widget_choice *choice = self->choice;

	if (idx >= choice->cnt) {
		GP_WARN("Choice idx %zu out of %zu", idx, choice->cnt);
		return NULL;
	}

	return self->choice->choices[idx];
}

static size_t get(gp_widget *self, enum gp_widget_choice_op op)
{
	switch (op) {
	case GP_WIDGET_CHOICE_OP_SEL:
		return self->choice->sel;
	case GP_WIDGET_CHOICE_OP_CNT:
		return self->choice->cnt;
	}

	return 0;
}

static void set(gp_widget *self, size_t val)
{
	self->choice->sel = val;
}

static struct gp_widget_choice_ops choice_ops = {
	.get_choice = get_choice,
	.get = get,
	.set = set
};

enum keys {
	CHOICES,
	OPS,
	SELECTED,
};

static const gp_json_obj_attr attrs[] = {
	GP_JSON_OBJ_ATTR("choices", GP_JSON_ARR),
	GP_JSON_OBJ_ATTR("ops", GP_JSON_STR),
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

	ret->choice->choices = (void*)ret->choice->payload;
	ret->choice->ops = &choice_ops;
	ret->choice->cnt = choice_cnt;
	ret->choice->sel = 0;

	return ret;
}

static gp_widget *parse_choices(unsigned int widget_type,
                                gp_json_reader *json, gp_json_val *val)
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
                                      gp_json_reader *json, gp_json_val *val,
                                      gp_widget_json_ctx *ctx)
{
	size_t sel = 0;
	int sel_set = 0;
	gp_widget *ret = NULL;
	const void *ops = NULL;

	(void)ctx;

	GP_JSON_OBJ_FILTER(json, val, &obj_filter, gp_widget_json_attrs) {
		switch (val->idx) {
		case CHOICES:
			if (ret) {
				gp_json_warn(json, "Only one of 'choices' and 'ops' can be set!");
				continue;
			}

			ret = parse_choices(widget_type, json, val);
		break;
		case OPS:
			if (ret) {
				gp_json_warn(json, "Only one of 'choices' and 'ops' can be set!");
				continue;
			}

			ops = gp_widget_struct_addr(val->val_str, ctx);
                        if (!ops) {
	                        gp_json_warn(json, "No ops structure '%s' defined", val->val_str);
				continue;
			}

			ret = gp_widget_choice_new2(widget_type, ops);
		break;
		case SELECTED:
			if (val->val_int < 0)
				gp_json_warn(json, "Invalid value!");
			else
				sel = val->val_int;

			sel_set = 1;
		break;
		}
	}

	if (!ret) {
		gp_json_warn(json, "Neither of 'choices' nor 'ops' defined");
		return NULL;
	}

	if (!sel_set)
		return ret;

	if (ops && sel_set) {
		gp_json_warn(json, "Cannot set 'selected' with 'ops'");
		return ret;
	}

	if (sel >= ret->choice->cnt)
		gp_json_warn(json, "Invalid selected choice %zu", sel);
	else
		ret->choice->sel = sel;

	return ret;
}

gp_widget *gp_widget_choice_new(unsigned int widget_type,
                                const char *choices[],
                                size_t choice_cnt,
                                size_t selected)
{
	size_t payload_size = gp_string_arr_size(choices, choice_cnt);
	gp_widget *ret = alloc_choice(widget_type, payload_size, choice_cnt);

	if (!ret)
		return NULL;

	gp_string_arr_copy(choices, choice_cnt, ret->choice->payload);

	if (selected >= choice_cnt)
		GP_WARN("Invalid selected choice %zu, max=%zu", selected, choice_cnt);
	else
		ret->choice->sel = selected;

	return ret;
}

static inline void call_set_sel(gp_widget *self, size_t sel)
{
	struct gp_widget_choice *choice = self->choice;

	choice->ops->set(self, sel);
}

gp_widget *gp_widget_choice_new2(unsigned int widget_type,
                                 const struct gp_widget_choice_ops *ops)
{
	gp_widget *ret = gp_widget_new(widget_type, GP_WIDGET_CLASS_CHOICE, sizeof(struct gp_widget_choice));

	if (!ret)
		return NULL;

	ret->choice->ops = ops;

	size_t cnt = call_get_cnt(ret);
	size_t sel = call_get_sel(ret);

	if (cnt && sel >= cnt)
		GP_WARN("Invalid selected choice %zu cnt %zu", sel, cnt);

	return ret;
}

void gp_widget_choice_refresh(gp_widget *self)
{
	size_t cnt = call_get_cnt(self);
	size_t sel = call_get_sel(self);

	if (sel >= cnt)
		call_set_sel(self, cnt-1);

	gp_widget_resize(self);
	gp_widget_redraw(self);
}

size_t gp_widget_choice_cnt_get(gp_widget *self)
{
	GP_WIDGET_CLASS_ASSERT(self, GP_WIDGET_CLASS_CHOICE, 0);

	return call_get_cnt(self);
}

const char *gp_widget_choice_name_get(gp_widget *self, size_t idx)
{
	GP_WIDGET_CLASS_ASSERT(self, GP_WIDGET_CLASS_CHOICE, NULL);

	return call_get_choice(self, idx);
}

__attribute__((visibility ("hidden")))
void gp_widget_choice_sel_set_(gp_widget *self, size_t sel)
{
	size_t cnt = call_get_cnt(self);

	if (sel >= cnt) {
		GP_WARN("Selected choice %zu >= cnt %zu!", sel, cnt);
		return;
	}

	if (call_get_sel(self) == sel)
		return;

	call_set_sel(self, sel);
}

void gp_widget_choice_sel_set(gp_widget *self, size_t sel)
{
	GP_WIDGET_CLASS_ASSERT(self, GP_WIDGET_CLASS_CHOICE, );

	gp_widget_choice_sel_set_(self, sel);

	gp_widget_redraw(self);
}

size_t gp_widget_choice_sel_get(gp_widget *self)
{
	GP_WIDGET_CLASS_ASSERT(self, GP_WIDGET_CLASS_CHOICE, 0);

	return call_get_sel(self);
}
