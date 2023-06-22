//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2021 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGET_SPINBUTTON_H
#define GP_WIDGET_SPINBUTTON_H

#include <widgets/gp_widget_choice.h>

/**
 * @brief Allocates and initializes new spinbutton widget.
 *
 * Spinbutton is a subclass of the choice widget.
 *
 * @choices An array of strings describing available choices.
 * @cnt Size of the choices array.
 * @sel Initially selected choice.
 *
 * @return A spinbutton widget.
 */
static inline gp_widget *gp_widget_spinbutton_new(const char *choices[],
                                                  size_t cnt, size_t sel)
{
	return gp_widget_choice_new(GP_WIDGET_SPINBUTTON, choices, cnt, sel);
}


/**
 * @brief Creates a spinbutton widget based on widget ops.
 *
 * @widget_type A widget type.
 * @ops A pointer to the widget ops.
 *
 * @return A spinbutton widget.
 */
static inline gp_widget *
gp_widget_spinbutton_ops_new(const struct gp_widget_choice_ops *ops)
{
	return gp_widget_choice_ops_new(GP_WIDGET_SPINBUTTON, ops);
}

/**
 * @brief Creates a spinbutton widget based on a static array.
 *
 * @array A pointer an array.
 * @memb_cnt An array size, i.e. number of elements.
 * @memb_size An array member size, e.g. sizeof(struct foo)
 * @memb_off An offset of the string for the choice in the array, e.g.
 *           offsetof(struct foo, str_elem)
 * @sel A selected choice.
 * @flags If GP_WIDGET_CHOICE_COPY is set the choices are copied into a
 *        separate array, otherwise the original array pointer is stored and used each
 *        time widget is rendered.
 *
 * @return A spinbutton widget.
 */
static inline gp_widget *
gp_widget_spinbutton_arr_new(const void *array,
                             size_t memb_cnt, uint16_t memb_size,
                             uint16_t memb_off, size_t sel, int flags)
{
	return gp_widget_choice_arr_new(GP_WIDGET_SPINBUTTON, array,
                                        memb_cnt, memb_size, memb_off,
	                                sel, flags);
}

#endif /* GP_WIDGET_SPINBUTTON_H */
