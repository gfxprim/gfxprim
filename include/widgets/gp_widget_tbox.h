//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2021 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGET_TBOX_H
#define GP_WIDGET_TBOX_H

#include <core/gp_seek.h>

struct gp_widget_tbox {
	/*
	 * If not NULL the tbox can contain only characters from this
	 * string, this is used as a hint when minimal tbox size is
	 * accounted for.
	 */
	const char *filter;

	/*
	 * Delimiter list for double click selection.
	 *
	 * If NULL defaults to whitespaces.
	 */
	const char *delim;

	int hidden:1; /* password prompt */
	int alert:1;

	size_t max_size;
	size_t cur_pos;
	size_t size;
	char *buf;

	/* Selection */
	size_t sel_off;
	size_t sel_len;

	gp_widget_tattr tattr;

	size_t off_left;

	//TODO: Move to event state
	struct timeval last_click;

	char payload[];
};

/**
 * @brief Event sub_type for tbox widget events.
 */
enum gp_widget_tbox_event_type {
	/** Emitted when enter is presseed */
	GP_WIDGET_TBOX_TRIGGER,
	/** Emitted before text is entered into the textbox */
	GP_WIDGET_TBOX_FILTER,
	/** Emitted after text is entered */
	GP_WIDGET_TBOX_EDIT,
};

enum gp_widget_tbox_flags {
	GP_WIDGET_TBOX_HIDDEN = 0x01,
};

#define GP_TBOX_FILTER_INT "0123456789"
#define GP_TBOX_FILTER_HEX "0123456789abcdefABCDEF"

/**
 * @brief Allocate and initialize a tbox widget.
 *
 * @text     Initial tbox text.
 * @len      Expected text length, used for widget size computation.
 * @max_len  Maximal number of characters, can be used as a limit.
 * @filter   If set only characters from the string can be typed into the tbox.
 * @on_event Callback called on tbox event, e.g. key press.
 * @priv     User private pointer.
 * @flags    Mask of enum gp_widget_tbox_flags.
 *
 * @returns A tbox widget.
 */
gp_widget *gp_widget_tbox_new(const char *text, gp_widget_tattr tattr,
                              unsigned int len, unsigned int max_len,
			      const char *filter, int flags,
                              int (*on_event)(gp_widget_event *),
                              void *priv);

/**
 * @brief A printf-like function to set a tbox value.
 *
 * @self A tbox widget.
 * @fmt A printf-like format string.
 * @... A printf-like parameters.
 *
 * @return A tbox string lenght or -1 on a failure.
 */
int gp_widget_tbox_printf(gp_widget *self, const char *fmt, ...)
                          __attribute__((format (printf, 2, 3)));


/**
 * @brief Sets a tbox text.
 *
 * @self A tbox widget.
 * @str A string.
 */
void gp_widget_tbox_set(gp_widget *self, const char *str);

/**
 * @brief Clears the tbox text.
 *
 * @self A tbox widget.
 */
void gp_widget_tbox_clear(gp_widget *self);

/**
 * @brief Returns a tbox string.
 *
 * @self A tbox widget.
 * @return Texbox widget string.
 */
const char *gp_widget_tbox_text(gp_widget *self);


/**
 * @brief Returns if tbox is empty.
 *
 * @self A tbox widget.
 * @return Non-zero if tbox is empty zero otherwise.
 */
static inline int gp_widget_tbox_is_empty(gp_widget *self)
{
	const char *text = gp_widget_tbox_text(self);

	if (!text || !text[0])
		return 1;

	return 0;
}

/**
 * @brief Returns current cursor postion.
 *
 * @self A tbox widget.
 * @return Current cursor position.
 */
size_t gp_widget_tbox_cursor_get(gp_widget *self);

/**
 * @brief Moves a cursor to a defined position.
 *
 * The whence works exactly as it would have for lseek(2). E.g. to move the
 * cursor one character back the whence would be set to GP_SEEK_CUR and off
 * to -1.
 *
 * @self A tbox widget.
 * @off An offset.
 * @whence Whence for the offset.
 */
void gp_widget_tbox_cursor_set(gp_widget *self, ssize_t off,
                               enum gp_seek_whence whence);

/**
 * @brief Inserts a string into a tbox.
 *
 * The whence works exactly as it would have for lseek(2).
 *
 * If we are inserting a text before cursor, the cursor moves as well. If text
 * is inserted after cursor, the cursor stays on its position.
 *
 * @self A tbox widget.
 * @off An offset.
 * @whence A whence for the offset.
 * @str A string.
 */
void gp_widget_tbox_ins(gp_widget *self, ssize_t off,
                        enum gp_seek_whence whence, const char *str);

/**
 * @brief Appends a string to the textbox.
 *
 * @self A tbox widget.
 * @str A string to append.
 */
static inline void gp_widget_tbox_append(gp_widget *self, const char *str)
{
	gp_widget_tbox_ins(self, 0, GP_SEEK_CUR, str);
}

/**
 * @brief Deletes a len characters from a tbox.
 *
 * The whence works exactly as it would have for lseek(2).
 *
 * If we are deleting a text before cursor, the cursor moves as well. If text
 * is deleted after cursor, the cursor stays on its position.
 *
 * @self A tbox widget.
 * @off An offset.
 * @whence A whence for the offset.
 * @len A number of characters to delete.
 */
void gp_widget_tbox_del(gp_widget *self, ssize_t off,
                        enum gp_seek_whence whence, size_t len);

/**
 * @brief Sets selection.
 *
 * When selection is set the cursor is moved to the end of the selection.
 *
 * @self A tbox widget.
 * @off An offset.
 * @whence A whence for the offset.
 * @len A selection length.
 */
void gp_widget_tbox_sel_set(gp_widget *self, ssize_t off,
                            enum gp_seek_whence whence, size_t len);

/**
 * @brief Selects whole textbox text.
 *
 * When selection is set the cursor is moved to the end of the selection.
 *
 * @self A tbox widget.
 */
void gp_widget_tbox_sel_all(gp_widget *self);

/**
 * @brief Clears selection.
 *
 * @self A tbox widget.
 */
void gp_widget_tbox_sel_clr(gp_widget *self);

/**
 * @brief Deletes selected characters.
 *
 * @self A tbox widget.
 */
void gp_widget_tbox_sel_del(gp_widget *self);

/**
 * @brief Returns selection lenght.
 *
 * Returns 0 when no text is selected.
 *
 * @self A tbox widget.
 * @return A selection length.
 */
size_t gp_widget_tbox_sel_len(gp_widget *self);

/**
 * @brief Returns selection offset, i.e. first character.
 *
 * Returns 0 when no text is selected.
 *
 * @self A tbox widget.
 * @return A selection offset.
 */
size_t gp_widget_tbox_sel_off(gp_widget *self);

/**
 * @brief Returns if text is selected.
 *
 * @return Non-zero if text is selected.
 */
static inline int gp_widget_tbox_sel(gp_widget *self)
{
	return !!gp_widget_tbox_sel_len(self);
}

#endif /* GP_WIDGET_TBOX_H */
