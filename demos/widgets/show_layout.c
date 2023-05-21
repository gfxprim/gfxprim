//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2023 Cyril Hrubis <metan@ucw.cz>

 */

#include <gfxprim.h>

int on_event(gp_widget_event *ev)
{
	gp_widget_event_dump(ev);
	return 0;
}

static int screenshot_callback(gp_task *self)
{
	const char *fname = self->priv;
	const gp_pixmap *pixmap = gp_widget_render_buffer();

	printf("Saving screenshot to '%s'\n", fname);

	gp_save_image(pixmap, fname, NULL);

	return 0;
}

struct gp_task screenshot_task = {
	.id = "screenshot",
	.prio = 1,
	.callback = screenshot_callback,
};

int main(int argc, char *argv[])
{
	gp_widgets_getopt(&argc, &argv);

	if (!argv[0]) {
		fprintf(stderr, "usage: show_layout test_layouts/foo.json [output_image]\n");
		return 1;
	}

	gp_widget *layout = gp_widget_layout_json(argv[0], NULL, NULL);
	if (!layout) {
		fprintf(stderr, "Layout cannot be loaded!\n");
		return 1;
	}

	if (argv[1]) {
		screenshot_task.priv = argv[1];
		gp_widgets_task_ins(&screenshot_task);
	}

	gp_widgets_main_loop(layout, "Layout", NULL, 0, NULL);

	return 0;
}
