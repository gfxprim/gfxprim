//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGET_TABS_H__
#define GP_WIDGET_TABS_H__

struct gp_widget_tab {
	char *label;
	gp_widget *widget;
};

struct gp_widget_tabs {
	unsigned int active_tab;

	int title_focused:1;
	int widget_focused:1;

	struct gp_widget_tab *tabs;

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

/**
 * @brief Adds a tab at an offset.
 *
 * @self A tabs widget.
 * @off An offset.
 * @label Tabs label.
 * @child A tab child, may be NULL.
 */
void gp_widget_tabs_add(gp_widget *self, unsigned int off,
                        const char *label, gp_widget *child);

/**
 * @brief Appends a tab.
 *
 * @self A tabs widget.
 * @label Tabs label.
 * @child A tab child, may be NULL.
 */
void gp_widget_tabs_append(gp_widget *self,
                           const char *label, gp_widget *child);

/**
 * @brief Remove a tab at an offset.
 *
 * The tabs child widget is destroyed by gp_widget_free().
 *
 * @self A tabs widget.
 * @off An offset.
 */
void gp_widget_tabs_rem(gp_widget *self, unsigned int off);

/**
 * @brief Returns active tab index.
 *
 * @self A tabs widget.
 *
 * @return An active tab index.
 */
unsigned int gp_widget_tabs_get_active(gp_widget *self);

/**
 * @brief Set active tab.
 *
 * @self A tabs widget.
 * @tab A tab index.
 */
void gp_widget_tabs_set_active(gp_widget *self, unsigned int tab);

#endif /* GP_WIDGET_TABS_H__ */
