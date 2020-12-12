//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGET_CHOICE_H__
#define GP_WIDGET_CHOICE_H__

struct gp_widget_choice {
	unsigned int sel;
	unsigned int max;
	char **choices;

	char payload[];
};

/**
 * @brief Allocates and initializes new choice widget.
 *
 * @choices An array of strings describing available choices.
 * @choice_cnt Size of the choices array.
 * @selected Initially selected choice.
 * @on_event An event handler.
 * @priv An user private pointer.
 *
 * @return A choice widget.
 */
gp_widget *gp_widget_choice_new(const char *choices[],
                                unsigned int choice_cnt,
                                unsigned int selected,
                                int (*on_event)(gp_widget_event *self),
                                void *priv);

/**
 * @brief Sets a selected choice in the choice widget.
 *
 * @self A choice widget.
 * @sel A choice to select.
 */
void gp_widget_choice_set(gp_widget *self, unsigned int sel);

/**
 * @brief Returns a selected choice.
 *
 * @self A choice widget.
 *
 * @return Currently selected choice widget.
 */
unsigned int gp_widget_choice_get(gp_widget *self);

#endif /* GP_WIDGET_CHOICE_H__ */
