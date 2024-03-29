//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2023 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGET_CHOICE_H
#define GP_WIDGET_CHOICE_H

enum gp_widget_choice_op {
	GP_WIDGET_CHOICE_OP_SEL,
	GP_WIDGET_CHOICE_OP_CNT,
};

typedef struct gp_widget_choice_ops {
	const char *(*get_choice)(gp_widget *self, size_t idx);
	size_t (*get)(gp_widget *self, enum gp_widget_choice_op op);
	void (*set)(gp_widget *self, size_t val);
} gp_widget_choice_ops;

/**
 * An array description to take the choices from along with ops to operate on
 * the description.
 */
typedef struct gp_widget_choice_arr {
	const void *ptr;
	size_t memb_cnt;
	uint16_t memb_size;
	uint16_t memb_off;
} gp_widget_choice_arr;

extern const gp_widget_choice_ops gp_widget_choice_arr_ops;

/**
 * Choice description which could be passed to the JSON loader.
 */
typedef struct gp_widget_choice_desc {
	const gp_widget_choice_ops *ops;
	union {
		void *ops_priv;
		const char *const *choices;
		gp_widget_choice_arr *arr;
	};
} gp_widget_choice_desc;

enum gp_widget_choice_flags {
	GP_WIDGET_CHOICE_COPY = 1,
};

struct gp_widget_choice {
	/* The actual widget data getters/setters */
	const struct gp_widget_choice_ops *ops;

	/*
	 * Optional storage for cnt and sel, can be utilized by the
	 * setters/getters from ops.
	 */
	size_t cnt;
	size_t sel;

	/* Previously selected choice can be queried in the widget callback */
	size_t prev_sel;

	/* Pointer to a widget data */
	union {
		void *ops_priv;
		char **choices;
		struct gp_widget_choice_arr *arr;
	};

	char payload[];
};

/**
 * @brief Allocates and initializes new choice widget.
 *
 * @widget_type A widget type.
 * @choices An array of strings describing available choices.
 * @cnt Size of the choices array.
 * @sel Initially selected choice.
 *
 * @return A choice widget.
 */
gp_widget *gp_widget_choice_new(unsigned int widget_type,
                                const char *choices[],
                                size_t cnt, size_t sel);

/**
 * @brief Creates a choice widget based on widget ops.
 *
 * @widget_type A widget type.
 * @ops A pointer to the widget ops.
 *
 * @return A choice widget.
 */
gp_widget *gp_widget_choice_ops_new(unsigned int widget_type,
                                    const struct gp_widget_choice_ops *ops);


/**
 * @brief Creates a choice widget based on a static array.
 *
 * @array A pointer an array.
 * @memb_cnt An array size, i.e. number of elements.
 * @memb_size An array member size, e.g. sizeof(struct foo)
 * @memb_off An offset of the string for the choice in the array, e.g.
 *           offsetof(struct foo, str_elem)
 * @sel A selected choice.
 * @flags If GP_WIDGET_CHOICE_COPY is set the choices are copied into a
 *        separate array, otherwise the original array pointer is stored and used each
 *        time widget is rendered.
 *
 * @return A choice widget.
 */
gp_widget *gp_widget_choice_arr_new(unsigned int widget_type, const void *array,
                                    size_t memb_cnt, uint16_t memb_size,
                                    uint16_t memb_off, size_t sel, int flags);

/**
 * @brief Request update after the choices has been changed.
 *
 * @self A choice widget.
 *
 * Causes the choice widget to be resized and rerendered.
 */
void gp_widget_choice_refresh(gp_widget *self);

/**
 * @brief Returns the number of choices to choose from.
 *
 * @self A choice widget.
 * @return A number of choices.
 */
size_t gp_widget_choice_cnt_get(gp_widget *self);

/**
 * @brief Sets a selected choice in the choice widget.
 *
 * @self A choice widget.
 * @sel A choice to select.
 */
void gp_widget_choice_sel_set(gp_widget *self, size_t sel);

/**
 * @brief Returns a selected choice.
 *
 * @self A choice widget.
 * @return Currently selected choice widget.
 */
size_t gp_widget_choice_sel_get(gp_widget *self);

/**
 * @brief Returns previously selected choice.
 *
 * The value is valid only after choice was changed from the UI, i.e. can be
 * used in the widget event callback to get the previosly selected choice.
 *
 * @self A choice widget.
 * @return A previously selected choice.
 */
size_t gp_widget_choice_prev_sel_get(gp_widget *self);

/**
 * @brief Returns a choice value for a given index.
 *
 * @self A choice widget.
 * @idx An choice index.
 *
 * @return A choice value.
 */
const char *gp_widget_choice_name_get(gp_widget *self, size_t idx);

/**
 * @brief Returns name of selected choice.
 *
 * @self A choice widget.
 */
static inline const char *gp_widget_choice_sel_name_get(gp_widget *self)
{
	return gp_widget_choice_name_get(self, gp_widget_choice_sel_get(self));
}

/**
 * @brief Parses JSON into a choice widget.
 *
 * @widget_type A widget type.
 * @json A JSON widget.
 * @uids A pointer to a hash table.
 *
 * @return A choice widget.
 */
gp_widget *gp_widget_choice_from_json(unsigned int widget_type,
                                      gp_json_reader *json, gp_json_val *val,
                                      gp_widget_json_ctx *ctx);

#endif /* GP_WIDGET_CHOICE_H */
