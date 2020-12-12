//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGET_TABS_H__
#define GP_WIDGET_TABS_H__

struct gp_widget_tabs {
	unsigned int active_tab;
	unsigned int count;

	int title_focused:1;
	int widget_focused:1;

	char **labels;
	struct gp_widget **widgets;

	char payload[];
};

/**
 * @brief Allocate and initialize new tabs widget.
 *
 * @tabs Number of tabs.
 * @active_tab Initially active tab.
 * @tab_labels Array of tab labels.
 *
 * @return A tabs widget.
 */
gp_widget *gp_widget_tabs_new(unsigned int tabs, unsigned int active_tab,
                              const char *tab_labels[]);

/**
 * @brief Puts a child into a tab.
 *
 * @self A tabs widget.
 * @tab Index of tab to put the child into.
 * @child A tab child widget.
 *
 * @return Previous child occupying the slot or NULL if it was empty.
 */
gp_widget *gp_widget_tabs_put(gp_widget *self, unsigned int tab,
                              gp_widget *child);

#endif /* GP_WIDGET_TABS_H__ */
