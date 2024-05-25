//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2021 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @file gp_widget_tbox.h
 * @brief A textbox widget.
 */

#ifndef GP_WIDGET_TBOX_H
#define GP_WIDGET_TBOX_H

#include <utils/gp_seek.h>
#include <utils/gp_utf_pos.h>

/**
 * @brief Text box type.
 *
 * Text box type modifies the way textbox is rendered or behaves.
 */
enum gp_widget_tbox_type {
	/** @brief No type, this is default. */
	GP_WIDGET_TBOX_NONE,
	/**
	 * @brief Hidden text, e.g. password.
	 *
	 * When set the text box shows asterisks or circles instead of
	 * characters.
	 */
	GP_WIDGET_TBOX_HIDDEN,
	/**
	 * @brief An URL.
	 *
	 * When set the selection delimiters is set to "/".
	 */
	GP_WIDGET_TBOX_URL,
	/**
	 * @brief A filesystem path.
	 *
	 * When set the selection delimiters is set to "/".
	 */
	GP_WIDGET_TBOX_PATH,
	/**
	 * @brief A file name.
	 *
	 * When set the selection delimiters is set to ".".
	 */
	GP_WIDGET_TBOX_FILENAME,
	/* @brief Last used type + 1. */
	GP_WIDGET_TBOX_MAX,
};

struct gp_widget_tbox {
	/* Text buffer */
	char *buf;
	size_t size;

	/* Help text shown when tbox is empty */
	char *help;

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
	 *
	 * This is set automatically by a certain tbox types.
	 */
	const char *delim;

	/* enum gp_widget_tbox_type */
	uint16_t type;

	uint16_t alert:1;
	uint16_t clear_on_input:1;

	size_t max_size;

	/* Cursor position */
	gp_utf8_pos cur_pos;
	gp_utf8_pos cur_pos_saved;
	/* Offset on left size, part of a string that is not shown */
	gp_utf8_pos off_left;

	/* Selection */
	gp_utf8_pos sel_left;
	gp_utf8_pos sel_right;

	gp_widget_tattr tattr;


	//TODO: Move to event state
	uint64_t last_click;
	uint32_t click_cursor_x;

	char payload[];
};

/**
 * @brief A gp_widget_event::sub_type for tbox widget events.
 */
enum gp_widget_tbox_event_type {
	/** @brief Emitted when enter is pressed. */
	GP_WIDGET_TBOX_TRIGGER,
	/**
	 * @brief Emitted to filter out a character before text was modified.
	 *
	 * The unicode character to be added to the text box is passed
	 * in gp_widget_event::val and the text box text is unmodified.
	 *
	 * By returning 0 from the gp_widget::on_event callback the change is
	 * approved and can proceed further.
	 */
	GP_WIDGET_TBOX_PRE_FILTER,
	/**
	 * @brief Emitted to filter out a character after text modified.
	 *
	 * The unicode character that has been added to the text box is passed
	 * in gp_widget_event::val and the text box text has this character
	 * inserted at the cursor position. The cursor is not moved to a next
	 * position until the filter approves the character insertion.
	 *
	 * By returning 0 from the gp_widget::on_event callback the change is
	 * approved and can proceed further. Otherwise the changes to text box
	 * text are undone.
	 */
	GP_WIDGET_TBOX_POST_FILTER,
	/**
	 * @brief Emitted after text is entered.
	 *
	 * This event is set each time a text box text is modified.
	 */
	GP_WIDGET_TBOX_EDIT,
	/**
	 * @brief Emitted when text box is modified.
	 *
	 * Emitted after text is set by:
	 *
	 * gp_widget_tbox_set() gp_widget_tbox_printf() gp_widget_tbox_clear()
	 *
	 * or modified by:
	 *
	 * gp_widget_tbox_ins() gp_widget_tbox_append() and gp_widget_tbox_del()
	 */
	GP_WIDGET_TBOX_SET,
	/**
	 * @brief Emitted before text is pasted.
	 *
	 * The text is then added one character at atime and filter events are
	 * generated for each character.
	 */
	GP_WIDGET_TBOX_PASTE,
};

#define GP_TBOX_FILTER_INT "0123456789"
#define GP_TBOX_FILTER_HEX "0123456789abcdefABCDEF"

/**
 * @brief Create a tbox widget.
 *
 * @param text Initial tbox text, pass NULL for empty text box.
 * @param tattr A text attribute e.g. monospace font.
 * @param len Expected text length, used for widget size computation.
 * @param max_len Maximal number of characters, can be used as a hard limit.
 * @param filter If set only characters from the string can be typed into the tbox.
 * @param type Text box type.
 *
 * @return A newly allocate and initialized tbox widget.
 */
gp_widget *gp_widget_tbox_new(const char *text, gp_widget_tattr tattr,
                              unsigned int len, unsigned int max_len,
			      const char *filter, enum gp_widget_tbox_type type);

/**
 * @brief A printf-like function to set a tbox text.
 *
 * @param self A tbox widget.
 * @param fmt A printf-like format string.
 * @param ... A printf-like parameters.
 *
 * @return A tbox string lenght or -1 on a failure.
 */
int gp_widget_tbox_printf(gp_widget *self, const char *fmt, ...)
                          __attribute__((format (printf, 2, 3)));


/**
 * @brief Sets a tbox text.
 *
 * @param self A tbox widget.
 * @param str A string.
 */
void gp_widget_tbox_set(gp_widget *self, const char *str);

/**
 * @brief Clears the tbox text.
 *
 * @param self A tbox widget.
 */
void gp_widget_tbox_clear(gp_widget *self);

/**
 * @brief Returns a tbox string.
 *
 * @param self A tbox widget.
 *
 * @return Texbox widget string.
 */
const char *gp_widget_tbox_text(gp_widget *self);


/**
 * @brief Returns true if tbox is empty.
 *
 * @param self A tbox widget.
 *
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
 * @param self A tbox widget.
 *
 * @return Current cursor position.
 */
gp_utf8_pos gp_widget_tbox_cursor_get(gp_widget *self);

/**
 * @brief Moves a cursor to a defined position.
 *
 * The whence works exactly as it would have for lseek(2). E.g. to move the
 * cursor one character back the whence would be set to GP_SEEK_CUR and off
 * to -1.
 *
 * @param self A tbox widget.
 * @param off An offset.
 * @param whence Whence for the offset.
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
 * @param self A tbox widget.
 * @param off An offset.
 * @param whence A whence for the offset.
 * @param str A string.
 */
void gp_widget_tbox_ins(gp_widget *self, ssize_t off,
                        enum gp_seek_whence whence, const char *str);

/**
 * @brief Appends a string to the textbox.
 *
 * @param self A tbox widget.
 * @param str A string to append.
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
 * @param self A tbox widget.
 * @param off An offset.
 * @param whence A whence for the offset.
 * @param len A number of characters to delete.
 */
void gp_widget_tbox_del(gp_widget *self, ssize_t off,
                        enum gp_seek_whence whence, size_t len);

/**
 * @brief Sets selection.
 *
 * When selection is set the cursor is moved to the end of the selection.
 *
 * @param self A tbox widget.
 * @param off An offset.
 * @param whence A whence for the offset.
 * @param len A selection length.
 */
void gp_widget_tbox_sel_set(gp_widget *self, ssize_t off,
                            enum gp_seek_whence whence, size_t len);

/**
 * @brief Selects whole textbox text.
 *
 * When selection is set the cursor is moved to the end of the selection.
 *
 * @param self A tbox widget.
 */
void gp_widget_tbox_sel_all(gp_widget *self);

/**
 * @brief Clears selection.
 *
 * @param self A tbox widget.
 */
void gp_widget_tbox_sel_clr(gp_widget *self);

/**
 * @brief Deletes selected characters.
 *
 * @param self A tbox widget.
 */
void gp_widget_tbox_sel_del(gp_widget *self);

/**
 * @brief Returns selection lenght.
 *
 * @param self A tbox widget.
 *
 * @return A text selection lenght, 0 when no text is selected.
 */
gp_utf8_pos gp_widget_tbox_sel_len(gp_widget *self);

/**
 * @brief Returns selection offset, i.e. first character.
 *
 * @param self A tbox widget.
 * @return A text selection offset, 0 when no text is selected.
 */
gp_utf8_pos gp_widget_tbox_sel_off(gp_widget *self);

/**
 * @brief Returns true if text is selected.
 *
 * @param self A tbox widget.
 *
 * @return Non-zero if text is selected.
 */
static inline int gp_widget_tbox_sel(gp_widget *self)
{
	return !!gp_widget_tbox_sel_len(self).bytes;
}

/**
 * @brief Sets textbox selection delimiters.
 *
 * Any of the characters from delim will limit selection on double click.
 *
 * Default delimiters are whitespaces. For some textbox types delimiters are
 * set as well e.g. for GP_WIDGET_TYPE_PATH the delimiters are set to "/".
 *
 * @param self A tbox widget.
 * @param delim A string containing delimiters.
 */
void gp_widget_tbox_sel_delim_set(gp_widget *self, const char *delim);

/**
 * @brief Sets textbox type.
 *
 * Textbox type changes default behaviors, e.g. if type is set to hidden
 * asterisks are shown instead of letters.
 *
 * Other types set the selection delimiter lists, etc.
 *
 * @param self A tbox widget.
 * @param type A tbox type.
 */
void gp_widget_tbox_type_set(gp_widget *self, enum gp_widget_tbox_type type);

/**
 * @brief Sets textbox help text.
 *
 * Passing NULL as text will clear the help text.
 *
 * The help is shown as gray text inside the textbox when it's empty.
 *
 * @param self A tbox widget.
 * @param help An utf8 help text.
 */
void gp_widget_tbox_help_set(gp_widget *self, const char *help);

/**
 * @brief Sets one time flag that clears the text on next input event.
 *
 * @param self A tbox widget.
 */
void gp_widget_tbox_clear_on_input(gp_widget *self);

#endif /* GP_WIDGET_TBOX_H */
