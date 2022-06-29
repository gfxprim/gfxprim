//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2021 Cyril Hrubis <metan@ucw.cz>

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

enum gp_widget_tabs_event_type {
	GP_WIDGET_TABS_DEACTIVATED, /* A tab is deactivated */
	GP_WIDGET_TABS_ACTIVATED, /* A tab activated */
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
gp_widget *gp_widget_tabs_new(unsigned int tabs_cnt, unsigned int active_tab,
                              const char *tab_labels[], int flags);

/**
 * @brief Returns number of tabs.
 *
 * @self A tabs widget.
 *
 * @return A number of tabs.
 */
unsigned int gp_widget_tabs_cnt(gp_widget *self);

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
 * @brief Removes child from a tab and returns pointer to it.
 *
 * @self A tabs widget.
 * @tab Tab position.
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
 * @self A tabs widget.
 * @tab Index of tab to put the child into.
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
 * @self A tabs widget.
 * @tab An offset.
 * @label Tabs label.
 * @child A tab child, may be NULL.
 */
void gp_widget_tabs_tab_ins(gp_widget *self, unsigned int tab,
                            const char *label, gp_widget *child);

/**
 * @brief Appends a tab at the end.
 *
 * @self A tabs widget.
 * @label Tabs label.
 * @child A tab child, may be NULL.
 *
 * @return Index of the appended tab.
 */
unsigned int gp_widget_tabs_tab_append(gp_widget *self,
                                       const char *label, gp_widget *child);

/**
 * @brief Appends a tab at the begining.
 *
 * @self A tabs widget.
 * @label Tabs label.
 * @child A tab child, may be NULL.
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
 * @self A tabs widget.
 * @off An offset.
 *
 * @return A tab child.
 */
gp_widget *gp_widget_tabs_tab_rem(gp_widget *self, unsigned int tab);

/**
 * @brief Remove a tab identified by a child widget.
 *
 * @self A tabs widget.
 * @off A tab child widget.
 *
 * @return Zero on success non-zero if child was not found.
 */
int gp_widget_tabs_tab_rem_by_child(gp_widget *self, gp_widget *child);

/**
 * @brief Delete a tab identified by a child widget.
 *
 * @self A tabs widget.
 * @off A tab child widget.
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
 * @self A tabs widget.
 * @off An offset.
 */
static inline void gp_widget_tabs_tab_del(gp_widget *self, unsigned int tab)
{
	gp_widget *ret = gp_widget_tabs_tab_rem(self, tab);

	gp_widget_free(ret);
}

/**
 * @brief Returns active tab index.
 *
 * @self A tabs widget.
 *
 * @return An active tab index.
 */
unsigned int gp_widget_tabs_active_get(gp_widget *self);

/**
 * @brief Returns active tab child widget.
 *
 * @self A tabs widget.
 *
 * @return An active tab child widget.
 */
gp_widget *gp_widget_tabs_active_child_get(gp_widget *self);

/**
 * @brief Set active tab.
 *
 * @self A tabs widget.
 * @tab A tab index.
 */
void gp_widget_tabs_active_set(gp_widget *self, unsigned int tab);

/**
 * @brief Set active tab position relative to the currently active tab.
 *
 * @self A tabs widget
 * @dir A direction to go to
 * @wrap_around Continue to beginning if we reach end and the other way around.
 */
void gp_widget_tabs_active_set_rel(gp_widget *self, int dir, int wrap_around);

/**
 * @brief Returns tab idx by child pointer.
 *
 * @self A tabs widget.
 * @child A tabs child.
 *
 * @return Tab index, if found, -1 otherwise.
 */
int gp_widget_tabs_tab_by_child(gp_widget *self, gp_widget *child);

/**
 * @brief Returns a tab label.
 *
 * @self A tabs widget.
 * @tab A tab index.
 *
 * @return A tab label or NULL if tab is out of range.
 */
const char *gp_widget_tabs_label_get(gp_widget *self, unsigned int tab);

/**
 * @brief Returns active tab label.
 *
 * @self A tabs widget.
 *
 * @return An active tab label.
 */
const char *gp_widget_tabs_active_label_get(gp_widget *self);

#endif /* GP_WIDGET_TABS_H */
