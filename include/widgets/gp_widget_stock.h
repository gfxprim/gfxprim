//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2024 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @file gp_widget_stock.h
 * @brief A stock image widget.
 *
 * Stock can either be used as widget to render a stock image in the
 * application layout. Or stock image can be embedded into other widgets, e.g.
 * button with a stock image.
 *
 * Stock widget JSON attributes
 * -----------------------------
 *
 * |   Attribute    |  Type  |   Default   | Description                                                |
 * |----------------|--------|-------------|------------------------------------------------------------|
 * |   **stock**    | string |             | Stock type, #gp_widget_stock_type.                         |
 * |  **min_size**  | string | 2*pad + asc | Minimal stock size parsed by gp_widget_size_units_parse(). |
 */

#ifndef GP_WIDGET_STOCK_H
#define GP_WIDGET_STOCK_H

#include <widgets/gp_widget_size_units.h>

/**
 * @brief Stock image type.
 */
typedef enum gp_widget_stock_type {
	/**
	 * @brief No stock image.
	 *
	 * Used in widgets to disable stock image.
	 */
	GP_WIDGET_STOCK_NONE = 0,
	/**
	 * @brief Empty stock.
	 *
	 * This is rendered as an empty i.e. background color rectangle.
	 */
	GP_WIDGET_STOCK_EMPTY,

	/** @brief Error message. */
	GP_WIDGET_STOCK_ERR,
	/** @brief Warning message. */
	GP_WIDGET_STOCK_WARN,
	/** @brief Info message. */
	GP_WIDGET_STOCK_INFO,
	/** @brief Warning message. */
	GP_WIDGET_STOCK_QUESTION,

	/** @brief Muted speaker. */
	GP_WIDGET_STOCK_SPEAKER_MUTE,
	/** @brief Speaker at minimal volume. */
	GP_WIDGET_STOCK_SPEAKER_MIN,
	/** @brief Speaker at middle volume. */
	GP_WIDGET_STOCK_SPEAKER_MID,
	/** @brief Speaker at maximal volume. */
	GP_WIDGET_STOCK_SPEAKER_MAX,
	/** @brief Increase speaker volume. */
	GP_WIDGET_STOCK_SPEAKER_INC,
	/** @brief Decrease speaker volume. */
	GP_WIDGET_STOCK_SPEAKER_DEC,

	/** @brief A hardware icon. */
	GP_WIDGET_STOCK_HARDWARE,
	/** @brief A software icon. */
	GP_WIDGET_STOCK_SOFTWARE,
	/** @brief A settings icon. */
	GP_WIDGET_STOCK_SETTINGS,
	/** @brief A home icon. */
	GP_WIDGET_STOCK_HOME,
	/** @brief A save icon. */
	GP_WIDGET_STOCK_SAVE,
	/** @brief A file icon. */
	GP_WIDGET_STOCK_FILE,
	/** @brief A directory icon. */
	GP_WIDGET_STOCK_DIR,
	/** @brief A new directory icon. */
	GP_WIDGET_STOCK_NEW_DIR,
	/** @brief A close icon. */
	GP_WIDGET_STOCK_CLOSE,
	/** @brief A refresh icon. */
	GP_WIDGET_STOCK_REFRESH,
	/** @brief A shuffle on icon. */
	GP_WIDGET_STOCK_SHUFFLE_ON,
	/** @brief A shuffle off icon. */
	GP_WIDGET_STOCK_SHUFFLE_OFF,
	/** @brief Repeat on icon. */
	GP_WIDGET_STOCK_REPEAT_ON,
	/** @brief Repeat off icon. */
	GP_WIDGET_STOCK_REPEAT_OFF,

	/** @brief A filter stock icon. */
	GP_WIDGET_STOCK_FILTER,

	/** @brief Arrow up. */
	GP_WIDGET_STOCK_ARROW_UP,
	/** @brief Arrow down. */
	GP_WIDGET_STOCK_ARROW_DOWN,
	/** @brief Arrow left. */
	GP_WIDGET_STOCK_ARROW_LEFT,
	/** @brief Arrow right. */
	GP_WIDGET_STOCK_ARROW_RIGHT,
	/** @brief Arrow rotate clock wise. */
	GP_WIDGET_STOCK_ROTATE_CW,
	/** @brief Arrow rotate counter clock wise. */
	GP_WIDGET_STOCK_ROTATE_CCW,

	/** @brief A day icon, a sun. */
	GP_WIDGET_STOCK_DAY,
	/** @brief A night icon, a moon. */
	GP_WIDGET_STOCK_NIGHT,
	/** @brief A star icon. */
	GP_WIDGET_STOCK_STAR,

	/** @brief A zoom icon. */
	GP_WIDGET_STOCK_ZOOM,
	/** @brief A zoom in icon. */
	GP_WIDGET_STOCK_ZOOM_IN,
	/** @brief A zoom out icon. */
	GP_WIDGET_STOCK_ZOOM_OUT,
	/** @brief A zoom fit icon. */
	GP_WIDGET_STOCK_ZOOM_FIT,
	/** @brief First unused stock type. */

	/** @brief Switch ON stock. */
	GP_WIDGET_STOCK_ON,
	/** @brief Switch OFF stock. */
	GP_WIDGET_STOCK_OFF,

	GP_WIDGET_STOCK_TYPE_MAX,
	/** @brief Invalid stock type, used by a gp_widget_stock_by_name(). */
	GP_WIDGET_STOCK_TYPE_INVALID = GP_WIDGET_STOCK_TYPE_MAX,

	/** @brief A stock focused flag, combined bitwise with type. */
	GP_WIDGET_STOCK_FOCUSED = 0x8000,
	/** @brief A type mask. */
	GP_WIDGET_STOCK_TYPE_MASK = (~(GP_WIDGET_STOCK_FOCUSED)),
} gp_widget_stock_type;

/**
 * @brief Masks out the stock type.
 *
 * @param type A type possibly combined with flags.
 */
#define GP_WIDGET_STOCK_TYPE(type) ((type) & GP_WIDGET_STOCK_TYPE_MASK)

/**
 * @brief Renders a stock image into a pixmap.
 *
 * @param pix A pixmap to render the image into.
 * @param type A stock image type.
 * @param x A x offset into the pixmap.
 * @param y A y offset into the pixmap.
 * @param w A stock image width.
 * @param h A stock image height.
 * @param bg_col A background color.
 * @param ctx A render context.
 */
void gp_widget_stock_render(gp_pixmap *pix, enum gp_widget_stock_type type,
                            gp_coord x, gp_coord y, gp_size w, gp_size h,
                            gp_pixel bg_col, const gp_widget_render_ctx *ctx);

/**
 * @brief Create a stock widget.
 *
 * @param type A stock image type.
 * @param min_size A minimal image size.
 *
 * @return A newly allocated and initialized stock widget.
 */
gp_widget *gp_widget_stock_new(enum gp_widget_stock_type type, gp_widget_size min_size);

/**
 * @brief Changes stock image type.
 *
 * @param self A stock widget.
 * @param type A new stock image type.
 */
void gp_widget_stock_type_set(gp_widget *self, enum gp_widget_stock_type type);

/**
 * @brief Checks for stock type validity.
 *
 * @param type A stock type.
 *
 * @return True if stock type is valid.
 */
bool gp_widget_stock_type_valid(gp_widget_stock_type type);

/**
 * @brief Parses stock type from a name.
 *
 * @param name A stock type name.
 *
 * @return A stock type id or GP_WIDGET_STOCK_TYPE_INVALID if name wasn't matched.
 */
gp_widget_stock_type gp_widget_stock_type_by_name(const char *name);

#endif /* GP_WIDGET_STOCK_H */
