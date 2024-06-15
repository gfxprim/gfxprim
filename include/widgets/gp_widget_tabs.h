//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2024 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @file gp_widget_tabs.h
 * @brief A tabs widget.
 *
 * Tabs widget JSON attributes
 * ----------------------------
 *
 * |  Attribute  | Type  | Default | Description                                                |
 * |-------------|-------|---------|------------------------------------------------------------|
 * | **labels**  | array |         | Array of string labels.                                    |
 * | **widgets** | array |         | Array of child widgets.                                    |
 * | **active**  | uint  |    0    | Active tab, index into the labels and widgets array.       |
 */

#ifndef GP_WIDGET_TABS_H
#define GP_WIDGET_TABS_H

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

/** @brief A gp_widget_event::sub_type for a tabs widget. */
enum gp_widget_tabs_event_type {
	/** @brief A tab is deactivated */
	GP_WIDGET_TABS_DEACTIVATED,
	/** @brief A tab activated */
	GP_WIDGET_TABS_ACTIVATED,
};

/**
 * @brief Allocates and initializes a new tabs widget.
 *
 * @param tabs_cnt Number of tabs.
 * @param active_tab Initially active tab.
 * @param tab_labels Array of tab labels.
 * @param flags Currently unused, must be 0.
 *
 * @return A tabs widget.
 */
gp_widget *gp_widget_tabs_new(unsigned int tabs_cnt, unsigned int active_tab,
                              const char *tab_labels[], int flags);

/**
 * @brief Returns number of tabs.
 *
 * @param self A tabs widget.
 *
 * @return A number of tabs.
 */
unsigned int gp_widget_tabs_cnt(gp_widget *self);

/**
 * @brief Puts a child into a tab.
 *
 * @param self A tabs widget.
 * @param tab Index of tab to put the child into.
 * @param child A tab child widget.
 *
 * @return Previous child occupying the slot or NULL if it was empty.
 */
gp_widget *gp_widget_tabs_put(gp_widget *self, unsigned int tab,
                              gp_widget *child);

/**
 * @brief Removes child from a tab and returns pointer to it.
 *
 * @param self A tabs widget.
 * @param tab Tab position.
 *
 * @return A child widget ocupying the slot or NULL if it's empty.
 */
static inline gp_widget *gp_widget_tabs_rem(gp_widget *self, unsigned int tab)
{
	return gp_widget_tabs_put(self, tab, NULL);
}

/**
 * @brief Returns a pointer to a child in a tab.
 *
 * @param self A tabs widget.
 * @param tab Index of tab to put the child into.
 *
 * @return A child widget ocupying the slot or NULL if it's empty.
 */
gp_widget *gp_widget_tabs_child_get(gp_widget *self, unsigned int tab);

static inline void gp_widget_tabs_del(gp_widget *self, unsigned int tab)
{
	gp_widget *ret = gp_widget_tabs_rem(self, tab);

	gp_widget_free(ret);
}

/**
 * @brief Adds a tab at an offset.
 *
 * @param self A tabs widget.
 * @param tab An offset.
 * @param label Tabs label.
 * @param child A tab child, may be NULL.
 */
void gp_widget_tabs_tab_ins(gp_widget *self, unsigned int tab,
                            const char *label, gp_widget *child);

/**
 * @brief Appends a tab at the end.
 *
 * @param self A tabs widget.
 * @param label Tabs label.
 * @param child A tab child, may be NULL.
 *
 * @return Index of the appended tab.
 */
unsigned int gp_widget_tabs_tab_append(gp_widget *self,
                                       const char *label, gp_widget *child);

/**
 * @brief Appends a tab at the begining.
 *
 * @param self A tabs widget.
 * @param label Tabs label.
 * @param child A tab child, may be NULL.
 */
static inline void gp_widget_tabs_tab_prepend(gp_widget *self,
                                              const char *label,
                                              gp_widget *child)
{
	return gp_widget_tabs_tab_ins(self, 0, label, child);
}

/**
 * @brief Remove a tab at position.
 *
 * @param self A tabs widget.
 * @param tab A tab index.
 *
 * @return A tab child.
 */
gp_widget *gp_widget_tabs_tab_rem(gp_widget *self, unsigned int tab);

/**
 * @brief Remove a tab identified by a child widget.
 *
 * @param self A tabs widget.
 * @param child A tab child widget.
 *
 * @return Zero on success non-zero if child was not found.
 */
int gp_widget_tabs_tab_rem_by_child(gp_widget *self, gp_widget *child);

/**
 * @brief Delete a tab identified by a child widget.
 *
 * @param self A tabs widget.
 * @param child A tab child widget.
 *
 * On successful deletion child is freed as well.
 *
 * @return Zero on success non-zero if child was not found.
 */
static inline int gp_widget_tabs_tab_del_by_child(gp_widget *self, gp_widget *child)
{
	if (!gp_widget_tabs_tab_rem_by_child(self, child))
		return 1;

	gp_widget_free(child);

	return 0;
}

/**
 * @brief Delete a tab at position.
 *
 * Removes a tab and frees the child widget.
 *
 * @param self A tabs widget.
 * @param tab A tab index.
 */
static inline void gp_widget_tabs_tab_del(gp_widget *self, unsigned int tab)
{
	gp_widget *ret = gp_widget_tabs_tab_rem(self, tab);

	gp_widget_free(ret);
}

/**
 * @brief Returns active tab index.
 *
 * @param self A tabs widget.
 *
 * @return An active tab index.
 */
unsigned int gp_widget_tabs_active_get(gp_widget *self);

/**
 * @brief Returns active tab child widget.
 *
 * @param self A tabs widget.
 *
 * @return An active tab child widget.
 */
gp_widget *gp_widget_tabs_active_child_get(gp_widget *self);

/**
 * @brief Set active tab.
 *
 * @param self A tabs widget.
 * @param tab A tab index.
 */
void gp_widget_tabs_active_set(gp_widget *self, unsigned int tab);

/**
 * @brief Set active tab position relative to the currently active tab.
 *
 * @param self A tabs widget
 * @param dir A direction to go to
 * @param wrap_around Continue to beginning if we reach end and the other way around.
 */
void gp_widget_tabs_active_set_rel(gp_widget *self, int dir, int wrap_around);

/**
 * @brief Returns tab idx by child pointer.
 *
 * @param self A tabs widget.
 * @param child A tabs child.
 *
 * @return Tab index, if found, -1 otherwise.
 */
int gp_widget_tabs_tab_by_child(gp_widget *self, gp_widget *child);

/**
 * @brief Returns a tab label.
 *
 * @param self A tabs widget.
 * @param tab A tab index.
 *
 * @return A tab label or NULL if tab is out of range.
 */
const char *gp_widget_tabs_label_get(gp_widget *self, unsigned int tab);

/**
 * @brief Returns active tab label.
 *
 * @param self A tabs widget.
 *
 * @return An active tab label.
 */
const char *gp_widget_tabs_active_label_get(gp_widget *self);

#endif /* GP_WIDGET_TABS_H */
