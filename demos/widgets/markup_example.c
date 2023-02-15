//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2019 Cyril Hrubis <metan@ucw.cz>

 */

#include <utils/gp_vec_str.h>
#include <widgets/gp_widgets.h>

#define MARKUP_STRING \
	"#Big Text#\n\n"\
	"Variables: {mmm} {0.0} m/s\n"\
	"~Second underline Line~\n"\
	"*Bold*_subscript_ and^superscript^\n"\
	"Escapes: \\_\\`\\{}\\*\\#\\~\n"\
	"*H_2_O N^2^ #H_2_O#*\n"\
	"*#Big Bold#small*"

int main(int argc, char *argv[])
{
	gp_widget *layout = gp_widget_grid_new(1, 2, 0);
	gp_widget *markup = gp_widget_markup_new(MARKUP_STRING, GP_MARKUP_GFXPRIM, 0);

	layout->align = GP_HFILL;
	markup->align = GP_HFILL;

	gp_widget_grid_put(layout, 0, 0, markup);
	gp_widget_grid_put(layout, 0, 1, gp_widget_markup_new("#Markup \\#2#", GP_MARKUP_GFXPRIM, 0));

	gp_widgets_main_loop(layout, "Markup test", NULL, argc, argv);

	return 0;
}
