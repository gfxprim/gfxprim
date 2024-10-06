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
	struct gp_widget_choice *choice = GP_WIDGET_PAYLOAD(self);

	if (idx >= choice->cnt) {
		GP_WARN("Choice idx %zu out of %zu", idx, choice->cnt);
		return NULL;
	}

	return choice->choices[idx];
}

static const char *get_arr_choice(gp_widget *self, size_t idx)
{
	struct gp_widget_choice *choice = GP_WIDGET_PAYLOAD(self);
	const char *const *res;

	if (idx >= choice->arr->memb_cnt) {
		GP_WARN("Choice idx %zu out of %zu", idx, choice->arr->memb_cnt);
		return NULL;
	}

	res = choice->arr->ptr + choice->arr->memb_size * idx + choice->arr->memb_off;

	return *res;
}

static size_t get(gp_widget *self, enum gp_widget_choice_op op)
{
	struct gp_widget_choice *choice = GP_WIDGET_PAYLOAD(self);

	switch (op) {
	case GP_WIDGET_CHOICE_OP_SEL:
		return choice->sel;
	case GP_WIDGET_CHOICE_OP_CNT:
		return choice->cnt;
	}

	return 0;
}

static size_t get_arr(gp_widget *self, enum gp_widget_choice_op op)
{
	struct gp_widget_choice *choice = GP_WIDGET_PAYLOAD(self);

	if (!choice->arr)
		return 0;

	switch (op) {
	case GP_WIDGET_CHOICE_OP_SEL:
		return choice->sel;
	case GP_WIDGET_CHOICE_OP_CNT:
		return choice->arr->memb_cnt;
	}

	return 0;
}

static void set(gp_widget *self, size_t val)
{
	struct gp_widget_choice *choice = GP_WIDGET_PAYLOAD(self);

	choice->sel = val;
}

static const struct gp_widget_choice_ops choice_ops = {
	.get_choice = get_choice,
	.get = get,
	.set = set
};

const struct gp_widget_choice_ops gp_widget_choice_arr_ops = {
	.get_choice = get_arr_choice,
	.get = get_arr,
	.set = set,
};

enum keys {
	CHOICES,
	DESC,
	SELECTED,
};

static const gp_json_obj_attr attrs[] = {
	GP_JSON_OBJ_ATTR_IDX(CHOICES, "choices", GP_JSON_ARR),
	GP_JSON_OBJ_ATTR_IDX(DESC, "desc", GP_JSON_STR),
	GP_JSON_OBJ_ATTR_IDX(SELECTED, "selected", GP_JSON_VOID),
};

static const gp_json_obj obj_filter = {
	.attrs = attrs,
	.attr_cnt = GP_ARRAY_SIZE(attrs),
};

static gp_widget *alloc_choice(enum gp_widget_type widget_type,
                               size_t payload_size, unsigned int choice_cnt)
{
	size_t size = sizeof(struct gp_widget_choice) + payload_size;
	gp_widget *ret;

	ret = gp_widget_new(widget_type, GP_WIDGET_CLASS_CHOICE, size);
	if (!ret)
		return NULL;

	struct gp_widget_choice *choice = GP_WIDGET_PAYLOAD(ret);

	choice->choices = (void*)choice->payload;
	choice->ops = &choice_ops;
	choice->cnt = choice_cnt;
	choice->sel = 0;

	return ret;
}

static inline void call_set_sel(gp_widget *self, size_t sel)
{
	struct gp_widget_choice *choice = GP_WIDGET_PAYLOAD(self);

	choice->ops->set(self, sel);
}

static gp_widget *parse_choices(enum gp_widget_type widget_type,
                                gp_json_reader *json, gp_json_val *val)
{
	gp_json_reader_state state = gp_json_reader_state_save(json);
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

	struct gp_widget_choice *choice = GP_WIDGET_PAYLOAD(ret);

	gp_json_reader_state_load(json, state);

	char *save = val->buf;

	val->buf = (void*)choice->payload + cnt * sizeof(void*);

	cnt = 0;

	GP_JSON_ARR_FOREACH(json, val) {
		if (val->type != GP_JSON_STR)
			continue;

		choice->choices[cnt++] = val->buf;
		val->buf += strlen(val->buf) + 1;
	}


	val->buf = save;

	return ret;
}

static size_t sel_by_str(gp_json_reader *json, gp_widget *choice, const char *sel)
{
	size_t i, cnt = call_get_cnt(choice);

	for (i = 0; i < cnt; i++) {
		const char *ch = call_get_choice(choice, i);

		if (!strcmp(ch, sel))
			return i;
	}

	gp_json_warn(json, "Invalid selected choice '%s' (does not exists)", sel);

	return 0;
}

gp_widget *gp_widget_choice_from_json(enum gp_widget_type widget_type,
                                      gp_json_reader *json, gp_json_val *val,
                                      gp_widget_json_ctx *ctx)
{
	size_t sel = 0;
	char *sel_str = NULL;
	int sel_set = 0;
	gp_widget *ret = NULL;
	gp_widget_choice_desc *desc = NULL;

	(void)ctx;

	GP_JSON_OBJ_FOREACH_FILTER(json, val, &obj_filter, gp_widget_json_attrs) {
		switch (val->idx) {
		case CHOICES:
			if (ret) {
				gp_json_warn(json, "Only one of 'choices' and 'ops' can be set!");
				continue;
			}

			ret = parse_choices(widget_type, json, val);
		break;
		case DESC:
			if (ret) {
				gp_json_warn(json, "Only one of 'choices' and 'ops' can be set!");
				continue;
			}

			desc = gp_widget_struct_addr(val->val_str, ctx);
                        if (!desc) {
	                        gp_json_warn(json, "No desc structure '%s' defined", val->val_str);
				continue;
			}

			ret = gp_widget_choice_ops_new(widget_type, desc->ops);
			if (ret) {
				struct gp_widget_choice *choice = GP_WIDGET_PAYLOAD(ret);
				choice->ops_priv = desc->ops_priv;
			}
		break;
		case SELECTED:
			if (sel_set) {
				gp_json_warn(json, "Duplicate selected value");
				continue;
			}

			if (val->type == GP_JSON_INT) {
				if (val->val_int < 0)
					gp_json_warn(json, "Invalid value!");
				else
					sel = val->val_int;
				sel_set = 1;
			} else if (val->type == GP_JSON_STR) {
				sel_str = strdup(val->val_str);
				sel_set = 1;
			} else {
				gp_json_warn(json, "Invalid value type, expected int or string!");
			}
		break;
		}
	}

	if (!ret) {
		gp_json_warn(json, "Neither of 'choices' nor 'desc' defined");
		return NULL;
	}

	if (sel_set) {
		size_t cnt = call_get_cnt(ret);

		if (sel_str) {
			sel = sel_by_str(json, ret, sel_str);
			free(sel_str);
		}

		if (sel < cnt) {
			call_set_sel(ret, sel);
		} else {
			gp_json_warn(json, "Invalid selected item %zu >= cnt %zu", sel, cnt);
		}
	}

	return ret;
}

gp_widget *gp_widget_choice_new(enum gp_widget_type widget_type,
                                const char *choices[],
                                size_t choice_cnt,
                                size_t selected)
{
	size_t payload_size = gp_string_arr_size(choices, choice_cnt);
	gp_widget *ret = alloc_choice(widget_type, payload_size, choice_cnt);

	if (!ret)
		return NULL;

	struct gp_widget_choice *choice = GP_WIDGET_PAYLOAD(ret);

	gp_string_arr_copy(choices, choice_cnt, choice->payload);

	if (selected >= choice_cnt)
		GP_WARN("Invalid selected choice %zu, max=%zu", selected, choice_cnt);
	else
		choice->sel = selected;

	return ret;
}

gp_widget *gp_widget_choice_arr_new(enum gp_widget_type widget_type, const void *array,
                                    size_t memb_cnt, uint16_t memb_size,
                                    uint16_t memb_off, size_t sel,
                                    enum gp_widget_choice_flags flags)
{
	gp_widget *ret;

	if (flags & ~GP_WIDGET_CHOICE_COPY) {
		GP_WARN("Invalid choice flags!");
		return NULL;
	}

	ret = gp_widget_new(widget_type, GP_WIDGET_CLASS_CHOICE,
	                    sizeof(struct gp_widget_choice) +
	                    sizeof(struct gp_widget_choice_arr));
	if (!ret)
		return NULL;

	struct gp_widget_choice *choice = GP_WIDGET_PAYLOAD(ret);

	choice->arr = (void*)choice->payload;

	choice->arr->memb_cnt = memb_cnt;
	choice->arr->memb_size = memb_size;
	choice->arr->memb_off = memb_off;
	choice->arr->ptr = array;

	if (sel >= memb_cnt)
		GP_WARN("Invalid selected choice %zu, max=%zu", sel, memb_cnt);
	else
		choice->sel = sel;

	choice->ops = &gp_widget_choice_arr_ops;

	return ret;
}

gp_widget *gp_widget_choice_ops_new(enum gp_widget_type widget_type,
                                    const struct gp_widget_choice_ops *ops)
{
	gp_widget *ret = gp_widget_new(widget_type, GP_WIDGET_CLASS_CHOICE, sizeof(struct gp_widget_choice));

	if (!ret)
		return NULL;

	struct gp_widget_choice *choice = GP_WIDGET_PAYLOAD(ret);

	choice->ops = ops;

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
	struct gp_widget_choice *choice = GP_WIDGET_PAYLOAD(self);

	if (sel >= cnt) {
		GP_WARN("Selected choice %zu >= cnt %zu!", sel, cnt);
		return;
	}

	size_t cur_sel = call_get_sel(self);

	if (cur_sel == sel)
		return;

	choice->prev_sel = cur_sel;

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

size_t gp_widget_choice_prev_sel_get(gp_widget *self)
{
	GP_WIDGET_CLASS_ASSERT(self, GP_WIDGET_CLASS_CHOICE, 0);
	struct gp_widget_choice *choice = GP_WIDGET_PAYLOAD(self);

	return choice->prev_sel;
}
