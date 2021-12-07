//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2021 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGET_CHOICE_H
#define GP_WIDGET_CHOICE_H

enum gp_widget_choice_op {
	GP_WIDGET_CHOICE_OP_SEL,
	GP_WIDGET_CHOICE_OP_CNT,
};

struct gp_widget_choice_ops {
	const char *(*get_choice)(gp_widget *self, size_t idx);
	size_t (*get)(gp_widget *self, enum gp_widget_choice_op op);
	void (*set)(gp_widget *self, enum gp_widget_choice_op op, size_t val);
};

struct gp_widget_choice {
	/* The actual widget data getters/setters */
	const struct gp_widget_choice_ops *ops;

	/*
	 * Cached choices count and selected choice
	 *
	 * These values does not change unless:
	 *
	 * - are explicitly set
	 * - refresh has been called
	 */
	size_t cnt;
	size_t sel;

	/* Pointer to a widget data */
	union {
		void *ops_priv;
		char **choices;
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
 * @brief Allocated and initializes new choice widget.
 *
 * @widget_type A widget type.
 * @get_choice An iterator over the choices.
 * @cnt A number of choices.
 * @sel A selected choice.
 *
 * @return A choice widget.
 */
gp_widget *gp_widget_choice_new2(unsigned int widget_type,
                                 const struct gp_widget_choice_ops *ops);

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
 *
 * @return Currently selected choice widget.
 */
size_t gp_widget_choice_sel_get(gp_widget *self);

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
 * @brief Parses JSON into a choice widget.
 *
 * @widget_type A widget type.
 * @json A JSON widget.
 * @uids A pointer to a hash table.
 *
 * @return A choice widget.
 */
gp_widget *gp_widget_choice_from_json(unsigned int widget_type,
                                      gp_json_buf *json, gp_json_val *val,
                                      gp_widget_json_ctx *ctx);

#endif /* GP_WIDGET_CHOICE_H */
