//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2024 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @file gp_widget_class_choice.h
 * @brief A choice class for widgets.
 */
#ifndef GP_WIDGET_CLASS_CHOICE_H
#define GP_WIDGET_CLASS_CHOICE_H

#include <core/gp_compiler.h>

/**
 * @brief A choice op for the get callback.
 */
enum gp_widget_choice_op {
	/** @brief Gets index of the selected choice. */
	GP_WIDGET_CHOICE_OP_SEL,
	/** @brief Gets the number of choices. */
	GP_WIDGET_CHOICE_OP_CNT,
};

/**
 * @brief A choice widget ops.
 *
 * These are callbacks called by the widget implementation to get the choice
 * values, selected choice and the number of choices.
 *
 * The application needs to call gp_widget_choice_refresh() when the state
 * changes, e.g. choice was added to or removed from the set.
 */
typedef struct gp_widget_choice_ops {
	/**
	 * @brief Returns a description for a choice.
	 *
	 * @param self A choice widget.
	 * @param idx A choice index.
	 *
	 * @return A choice description for a given index.
	 */
	const char *(*get_choice)(gp_widget *self, size_t idx);
	/**
	 * @brief Returns a selected choice or number of choices.
	 *
	 * @param self A choice widget.
	 * @param op A value to return.
	 *
	 * @return A value based on the op, either selected choice or number of
	 *         choices.
	 */
	size_t (*get)(gp_widget *self, enum gp_widget_choice_op op);
	/**
	 * @brief Sets the selected choice.
	 *
	 * @param self A choice widget.
	 * @param sel A selected choice.
	 */
	void (*set)(gp_widget *self, size_t sel);
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

	char payload[] GP_ALIGNED;
};

/**
 * @brief Allocates and initializes new choice widget.
 *
 * @param widget_type A widget type.
 * @param choices An array of strings describing available choices.
 * @param cnt Size of the choices array.
 * @param sel Initially selected choice.
 *
 * @return A choice widget.
 */
gp_widget *gp_widget_choice_new(enum gp_widget_type widget_type,
                                const char *choices[],
                                size_t cnt, size_t sel);

/**
 * @brief Creates a choice widget based on widget ops.
 *
 * @param widget_type A widget type.
 * @param ops A pointer to the widget ops.
 *
 * @return A choice widget.
 */
gp_widget *gp_widget_choice_ops_new(enum gp_widget_type widget_type,
                                    const struct gp_widget_choice_ops *ops);


/**
 * @brief Creates a choice widget based on a static array.
 *
 * @param widget_type A widget type.
 * @param array A pointer an array.
 * @param memb_cnt An array size, i.e. number of elements.
 * @param memb_size An array member size, e.g. sizeof(struct foo)
 * @param memb_off An offset of the string for the choice in the array, e.g.
 *                 offsetof(struct foo, str_elem)
 * @param sel A selected choice.
 * @param flags A bitwise combination of flags.
 *
 * @return A choice widget.
 */
gp_widget *gp_widget_choice_arr_new(enum gp_widget_type widget_type,
                                    const void *array,
                                    size_t memb_cnt, uint16_t memb_size,
                                    uint16_t memb_off, size_t sel,
                                    enum gp_widget_choice_flags flags);


/**
 * @brief Request update after the choices has been changed.
 *
 * Causes the choice widget to be resized and rerendered. Since there are cases
 * when the widget state is not stored in the widget, but rather feched by the
 * #gp_widget_choice_ops this call tells the widget to call the callback to
 * refresh the state.
 *
 * @param self A choice widget.
 */
void gp_widget_choice_refresh(gp_widget *self);

/**
 * @brief Returns the number of choices to choose from.
 *
 * @param self A choice widget.
 * @return A number of choices.
 */
size_t gp_widget_choice_cnt_get(gp_widget *self);

/**
 * @brief Sets a selected choice in the choice widget.
 *
 * @param self A choice widget.
 * @param sel A choice to select.
 */
void gp_widget_choice_sel_set(gp_widget *self, size_t sel);

/**
 * @brief Returns a selected choice.
 *
 * @param self A choice widget.
 *
 * @return Currently selected choice widget.
 */
size_t gp_widget_choice_sel_get(gp_widget *self);

/**
 * @brief Returns previously selected choice.
 *
 * The value is valid only after choice was changed from the UI, i.e. can be
 * used in the widget event callback to get the previosly selected choice.
 *
 * @param self A choice widget.
 *
 * @return A previously selected choice.
 */
size_t gp_widget_choice_prev_sel_get(gp_widget *self);

/**
 * @brief Returns a choice value for a given index.
 *
 * @param self A choice widget.
 * @param idx An choice index.
 *
 * @return A choice value.
 */
const char *gp_widget_choice_name_get(gp_widget *self, size_t idx);

/**
 * @brief Returns name of selected choice.
 *
 * @param self A choice widget.
 *
 * @return A selected choice name.
 */
static inline const char *gp_widget_choice_sel_name_get(gp_widget *self)
{
	return gp_widget_choice_name_get(self, gp_widget_choice_sel_get(self));
}

/**
 * @brief Parses JSON into a choice widget.
 *
 * @param widget_type A widget type.
 * @param json A JSON widget.
 * @param val A JSON value.
 * @param ctx A widget JSON loader context.
 *
 * @return A choice widget.
 */
gp_widget *gp_widget_choice_from_json(enum gp_widget_type widget_type,
                                      gp_json_reader *json, gp_json_val *val,
                                      gp_widget_json_ctx *ctx);

#endif /* GP_WIDGET_CLASS_CHOICE_H */
