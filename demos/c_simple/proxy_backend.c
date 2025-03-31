//SPDX-License-Identifier: LGPL-2.0-or-later
/*

   Copyright (c) 2019-2024 Cyril Hrubis <metan@ucw.cz>

 */

/*
 * This is a server part of proxy backend example, you need to start this
 * sever, then you can connect any backend or widget demo to this sever.
 */

#include <signal.h>
#include <gfxprim.h>
#include <sys/socket.h>

#include <backends/gp_proxy_proto.h>
#include <backends/gp_proxy_conn.h>
#include <backends/gp_proxy_shm.h>
#include <backends/gp_proxy_cli.h>

static gp_backend *backend;
static struct gp_proxy_shm *shm;
static gp_dlist clients;
static gp_proxy_cli *cli_shown;

static gp_pixel bg;
static gp_pixel fg;

static void redraw(void)
{
	gp_dlist_head *i;
	gp_fill(backend->pixmap, bg);

	gp_coord y = 20;
	gp_coord x = 20;
	gp_size spacing = 20;
	int n = 0;

	gp_print(backend->pixmap, NULL, x, y, GP_ALIGN_RIGHT|GP_VALIGN_BOTTOM,
	         fg, bg, "Connected clients: (press Ctrl+number to show a client)");

	y += spacing;

	GP_LIST_FOREACH(&clients, i) {
		gp_proxy_cli *cli = GP_LIST_ENTRY(i, struct gp_proxy_cli, head);
		gp_print(backend->pixmap, NULL, x, y, GP_ALIGN_RIGHT|GP_VALIGN_BOTTOM,
			 fg, bg, "%i: '%s'", n++, cli->name);
		y += spacing;
	}

	gp_backend_flip(backend);
}

static void shm_update(gp_proxy_cli *self, gp_coord x, gp_coord y, gp_size w, gp_size h)
{
	gp_size screen_h = backend->pixmap->h;

	if (self != cli_shown)
		return;

	if (h > screen_h) {
		GP_WARN("Invalid height");
		h = screen_h;
	}

	//TODO: Check SIZE!!!
	gp_blit_xywh_clipped(&shm->pixmap, x, y, w, h, backend->pixmap, x, y);

	gp_backend_update_rect_xywh(backend, x, y, w, h);

	gp_proxy_cli_rect_updated(self, x, y, w, h);
}

static void do_exit(void)
{
	gp_backend_exit(backend);
	exit(0);
}

/*
 * App resize handler, we have to wait for the client to unmap the memory
 * before we resize it, hence we have to wait for the application to ack the resize.
 */
static void on_unmap(gp_proxy_cli *self)
{
	if (self == cli_shown) {
		if (gp_proxy_shm_resize(shm, backend->pixmap->w, backend->pixmap->h) < 0)
			do_exit();

		gp_proxy_cli_send(cli_shown, GP_PROXY_MAP, &shm->path);
		gp_proxy_cli_send(cli_shown, GP_PROXY_PIXMAP, &shm->pixmap);
		gp_proxy_cli_send(cli_shown, GP_PROXY_SHOW, NULL);
	}
}

static void hide_client(void)
{
	if (!cli_shown)
		return;

	gp_proxy_cli_hide(cli_shown);

	cli_shown = NULL;
}

static void show_client(int pos)
{
	int n = 0;
	gp_dlist_head *i;

	GP_LIST_FOREACH(&clients, i) {
		if (n >= pos)
			break;
		n++;
	}

	if (!i)
		return;

	gp_proxy_cli *cli = GP_LIST_ENTRY(i, struct gp_proxy_cli, head);

	hide_client();

	struct gp_proxy_coord cur_pos = {
		.x = backend->event_queue->state.cursor_x,
		.y = backend->event_queue->state.cursor_y,
	};

	gp_proxy_cli_show(cli, shm, &cur_pos);

	cli_shown = cli;
}

static void resize_shown_client(void)
{
	if (!cli_shown) {
		if (gp_proxy_shm_resize(shm, backend->pixmap->w, backend->pixmap->h) < 0)
			do_exit();
		return;
	}

	gp_proxy_cli_send(cli_shown, GP_PROXY_UNMAP, NULL);
}

static int backend_event(gp_backend *b)
{
	gp_event *ev;

	while ((ev = gp_backend_ev_get(b))) {
		switch (ev->type) {
		case GP_EV_KEY:
			if (!gp_ev_any_key_pressed(ev,
			                           GP_KEY_LEFT_CTRL,
			                           GP_KEY_RIGHT_CTRL)) {
				goto to_cli;
			}

			if (ev->code != GP_EV_KEY_DOWN)
				goto to_cli;

			switch (ev->val) {
			case GP_KEY_ESC:
				do_exit();
			break;
			case GP_KEY_Q:
				if (cli_shown)
					gp_proxy_cli_send(cli_shown, GP_PROXY_EXIT, NULL);
			break;
			case GP_KEY_L:
				hide_client();
				redraw();
			break;
			case GP_KEY_1 ... GP_KEY_9:
				show_client(ev->val - GP_KEY_1 + 1);
			break;
			case GP_KEY_0:
				show_client(0);
			break;
			}

		break;
		case GP_EV_SYS:
			switch (ev->code) {
			case GP_EV_SYS_QUIT:
				do_exit();
			break;
			case GP_EV_SYS_RESIZE:
				gp_backend_resize_ack(b);
				redraw();
				gp_backend_flip(b);
				resize_shown_client();
				return 0;
			break;
			}
		break;
		}
to_cli:
		if (cli_shown)
			gp_proxy_cli_event(cli_shown, ev);
		else
			redraw();
	}


	return 0;
}

static void err_rem_cli(gp_fd *self)
{
	gp_backend_poll_rem(backend, self);
	close(self->fd);
	gp_proxy_cli_rem(&clients, self->priv);

	if (!cli_shown || self->priv == cli_shown) {
		cli_shown = NULL;
		redraw();
	}
}

static enum gp_poll_event_ret client_event(gp_fd *self)
{
	gp_proxy_msg *msg;

	if (gp_proxy_cli_read(self->priv)) {
		err_rem_cli(self);
		return 0;
	}

	for (;;) {
		if (gp_proxy_cli_msg(self->priv, &msg)) {
			err_rem_cli(self);
			return 0;
		}

		if (!msg)
			return 0;

		switch (msg->type) {
		case GP_PROXY_UNMAP:
			on_unmap(self->priv);
		break;
		case GP_PROXY_UPDATE:
			shm_update(self->priv,
			           msg->rect.rect.x, msg->rect.rect.y,
			           msg->rect.rect.w, msg->rect.rect.h);
		break;
		case GP_PROXY_NAME:
			if (!cli_shown)
				redraw();
		break;
		}
	}
}

static int client_add(gp_backend *backend, int fd)
{
	gp_proxy_cli *cli = gp_proxy_cli_add(&clients, fd);

	if (!cli)
		goto err0;

	cli->fd.event = client_event;
	cli->fd.priv = cli;

	gp_backend_poll_add(backend, &cli->fd);

	return 0;
err0:
	close(fd);
	return 1;
}

static enum gp_poll_event_ret server_event(gp_fd *self)
{
	int fd;

	while ((fd = accept(self->fd, NULL, NULL)) > 0) {
		struct gp_proxy_cli_init_ init = {
			.pixel_type = gp_backend_pixel_type(backend),
			.dpi = backend->dpi,
		};
		gp_proxy_send(fd, GP_PROXY_CLI_INIT, &init);

		client_add(backend, fd);
	}

	return 0;
}

static void print_help(void)
{
	printf("Keys\n");
	printf("----\n");
	printf("Ctrl + l ...... Lists connected clients\n");
	printf("Ctrl + 0 - 9 .. Selects backend\n");
	printf("Ctrl + q ...... Request client exit\n");
	printf("Ctrl + Esc .... Server exit\n");
}

int main(int argc, char *argv[])
{
	const char *backend_opts = NULL;
	int opt;

	print_help();

	signal(SIGPIPE, SIG_IGN);

	while ((opt = getopt(argc, argv, "b:h")) != -1) {
	switch (opt) {
		case 'b':
			backend_opts = optarg;
		break;
		case 'h':
			gp_backend_init_help();
		break;
		default:
			fprintf(stderr, "Invalid parameter '%c'", opt);
		}
	}

	backend = gp_backend_init(backend_opts, 0, 0, "proxy backend server");
	if (!backend) {
		fprintf(stderr, "Failed to initialize backend\n");
		return 1;
	}

	bg = 0;
	fg = 0xffffff;

	gp_size w = backend->pixmap->w;
	gp_size h = backend->pixmap->h;

	shm = gp_proxy_shm_init("/dev/shm/.proxy_backend", w, h, gp_backend_pixel_type(backend));
	if (!shm) {
		gp_backend_exit(backend);
		return 1;
	}

	redraw();

	int fd = gp_proxy_server_init(NULL);
	gp_fd server_fd = {
		.fd = fd,
		.event = server_event,
		.events = GP_POLLIN,
	};

	gp_backend_poll_add(backend, &server_fd);

	for (;;) {
		gp_backend_wait(backend);
		backend_event(backend);
	}

	return 0;
}
