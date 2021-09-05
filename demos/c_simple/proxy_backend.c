//SPDX-License-Identifier: LGPL-2.0-or-later
/*

   Copyright (c) 2019-2020 Cyril Hrubis <metan@ucw.cz>

 */

/*
 * This is a server part of proxy backend example, you need to start this
 * sever, then you can connect any backend demo to this sever.
 */

#include <signal.h>
#include <gfxprim.h>
#include <sys/socket.h>

#include <backends/gp_proxy_proto.h>
#include <backends/gp_proxy_conn.h>
#include <backends/gp_proxy_shm.h>
#include <backends/gp_proxy_cli.h>

static gp_backend *backend;
static const char *backend_opts = "x11";
static struct gp_proxy_shm *shm;
static struct gp_proxy_cli *clients;
static struct gp_proxy_cli *cli_shown;

static gp_pixel bg;
static gp_pixel fg;

static void redraw(void)
{
	struct gp_proxy_cli *i;
	gp_fill(backend->pixmap, bg);

	gp_coord y = 20;
	gp_coord x = 20;
	gp_size spacing = 20;
	int n = 0;

	gp_print(backend->pixmap, NULL, x, y, GP_ALIGN_RIGHT|GP_VALIGN_BOTTOM,
	         fg, bg, "Connected clients");

	y += spacing;

	for (i = clients; i; i = i->next) {
		gp_print(backend->pixmap, NULL, x, y, GP_ALIGN_RIGHT|GP_VALIGN_BOTTOM,
			 fg, bg, "%i: '%s'", n++, i->name);
		y += spacing;
	}

	gp_backend_flip(backend);
}

static void shm_update(struct gp_proxy_cli *self, gp_coord x, gp_coord y, gp_size w, gp_size h)
{
	gp_size screen_h = backend->pixmap->h;

	if (self != cli_shown)
		return;

	if (h > screen_h) {
		GP_WARN("Invalid height");
		h = screen_h;
	}

	printf("%i %i %u %u\n", x, y, w, h);

	//TODO: Check SIZE!!!
	gp_blit_xywh_clipped(&shm->pixmap, x, y, w, h, backend->pixmap, x, y);

	gp_backend_update_rect_xywh(backend, x, y, w, h);
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
static void on_unmap(struct gp_proxy_cli *self)
{
	if (self == cli_shown) {
		if (gp_proxy_shm_resize(shm, backend->pixmap->w, backend->pixmap->h) < 0)
			do_exit();

		gp_proxy_send(cli_shown->fd, GP_PROXY_MAP, &shm->path);
		gp_proxy_send(cli_shown->fd, GP_PROXY_PIXMAP, &shm->pixmap);
		gp_proxy_send(cli_shown->fd, GP_PROXY_SHOW, NULL);
	}
}

struct gp_proxy_cli_ops cli_ops = {
	.update = shm_update,
	.on_unmap = on_unmap,
};

static void hide_client(void)
{
	if (!cli_shown)
		return;

	gp_proxy_send(cli_shown->fd, GP_PROXY_HIDE, NULL);
	gp_proxy_send(cli_shown->fd, GP_PROXY_UNMAP, NULL);

	cli_shown = NULL;
}

static void show_client(int i)
{
	int n = 0;
	struct gp_proxy_cli *cli;

	for (cli = clients; cli && n < i; cli = cli->next)
		n++;

	if (!cli)
		return;

	hide_client();

	gp_proxy_send(cli->fd, GP_PROXY_MAP, &shm->path);
	gp_proxy_send(cli->fd, GP_PROXY_PIXMAP, &shm->pixmap);
	gp_proxy_send(cli->fd, GP_PROXY_SHOW, NULL);

	cli_shown = cli;
}

static void resize_shown_client(void)
{
	if (!cli_shown) {
		if (gp_proxy_shm_resize(shm, backend->pixmap->w, backend->pixmap->h) < 0)
			do_exit();
		return;
	}

	gp_proxy_send(cli_shown->fd, GP_PROXY_UNMAP, NULL);
}

static int backend_event(struct gp_fd *self, struct pollfd *pfd)
{
	gp_backend *b = self->priv;
	gp_event *ev;

	(void) pfd;

	while ((ev = gp_backend_poll_event(b))) {
		switch (ev->type) {
		case GP_EV_KEY:
			if (!gp_event_any_key_pressed(ev,
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
					gp_proxy_send(cli_shown->fd, GP_PROXY_EXIT, NULL);
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

struct gp_fds fds = GP_FDS_INIT;

static int client_event(struct gp_fd *self, struct pollfd *pfd)
{
	if (gp_proxy_cli_read(self->priv, &cli_ops)) {
		gp_proxy_cli_rem(&clients, self->priv);
		close(pfd->fd);

		if (self->priv == cli_shown) {
			cli_shown = NULL;
			redraw();
		}

		return 1;
	}

	return 0;
}

static int client_add(int fd)
{
	struct gp_proxy_cli *cli = gp_proxy_cli_add(&clients, fd);

	if (!cli)
		goto err0;

	if (gp_fds_add(&fds, fd, POLLIN, client_event, cli))
		goto err1;

	return 0;
err1:
	gp_proxy_cli_rem(&clients, cli);
err0:
	close(fd);
	return 1;
}

static int server_event(struct gp_fd *self, struct pollfd *pfd)
{
	(void) self;
	int fd;

	while ((fd = accept(pfd->fd, NULL, NULL)) > 0) {
		/*
		 * Pixel type has to be send first so that backend can return
		 * from init() function.
		 */
		gp_proxy_send(fd, GP_PROXY_PIXEL_TYPE, &backend->pixmap->pixel_type);

		client_add(fd);
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
	int opt;

	print_help();

	signal(SIGPIPE, SIG_IGN);

	while ((opt = getopt(argc, argv, "b:h")) != -1) {
	switch (opt) {
		case 'b':
			backend_opts = optarg;
		break;
		default:
			fprintf(stderr, "Invalid parameter '%c'", opt);
		}
	}

	backend = gp_backend_init(backend_opts, "proxy backend server");
	if (!backend) {
		fprintf(stderr, "Failed to initialize backend\n");
		return 1;
	}

	bg = 0;
	fg = 0xffffff;

	gp_size w = backend->pixmap->w;
	gp_size h = backend->pixmap->h;

	shm = gp_proxy_shm_init("/dev/shm/.proxy_backend", w, h, backend->pixmap->pixel_type);
	if (!shm) {
		gp_backend_exit(backend);
		return 1;
	}

	redraw();

	gp_fds_add(&fds, backend->fd, POLLIN, backend_event, backend);

	int server_fd = gp_proxy_server_init(NULL);

	gp_fds_add(&fds, server_fd, POLLIN, server_event, NULL);

	for (;;)
		gp_fds_poll(&fds, -1);

	return 0;
}
