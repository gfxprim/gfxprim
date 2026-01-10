// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2023 Erik LHC Hamera <y08192eh@slizka.chapadla.cz>
 * Copyright (C) 2024 Cyril Hrubis <metan@ucw.cz>
 */

#include <backends/gp_backend.h>
#include <core/gp_pixmap.h>
#include <core/gp_debug.h>

#include "gp_display_eink.h"
#include "gp_display_conn.h"

#include "gp_display_st7565.h"

static void st7565_exit(gp_backend *self)
{
	struct gp_display_spi *disp = GP_BACKEND_PRIV(self);

	gp_display_spi_cmd(disp, ST7565_DISPOFF);
	gp_display_spi_exit(disp);

	free(self->pixmap);
	free(self);
}

static void st7565_init(struct gp_display_spi *disp)
{
	gp_gpio_write(&disp->gpio_map->reset, 0);
	usleep(500000);
	gp_gpio_write(&disp->gpio_map->reset, 1);
	usleep(10);
	gp_display_spi_cmd(disp, ST7565_BIAS_1_7);

	gp_display_spi_cmd(disp, ST7565_ACD_NORMAL);
	gp_display_spi_cmd(disp, ST7565_COM_NORMAL);

	gp_display_spi_cmd(disp, ST7565_SETSTARTLINE);

	gp_display_spi_cmd(disp, ST7565_POWERCTRL | 0x04);
	usleep(50000);

	gp_display_spi_cmd(disp, ST7565_POWERCTRL | 0x06);
	usleep(50000);

	gp_display_spi_cmd(disp, ST7565_POWERCTRL | 0x07);
	usleep(10000);

	gp_display_spi_cmd(disp, ST7565_RES_RATIO | 0x06);

	gp_display_spi_cmd(disp, ST7565_DISPON);

	gp_display_spi_cmd(disp, ST7565_DISP_RAM);

	gp_display_spi_cmd(disp, ST7565_CONTRAST);
	gp_display_spi_cmd(disp, 0x0f);
}

static void st7565_repaint_full(gp_backend *self)
{
	struct gp_display_spi *disp = GP_BACKEND_PRIV(self);
	unsigned int y, p;

	/*
	 * Writing to the display is organized in character-lines 8 pixels high, while columns are autoincremented.
	 * Therefore bits from every byte ends in 8 adjacent graphic-lines in the same column which is not practical.
	 *
         * So the display is rotated in memory to have width 64 pixels which consists of 8 byte writes and height
	 * 128 pixels.
	 *
	 * But auto increment adds 1 to column (height in memory) after every byte write, so this algrithm fills
	 * a first 8 pixels (pixel0 - pixel7) from top to bottom, then pixel8 - pixel15 from top to bottom, etc.
	 *
	 * Note offset of line_bytes and increment of line_bytes in the second loop.
	 */
	for (p = 0; p < 8; p++) {
		gp_display_spi_cmd(disp, ST7565_SETPAGESTART | p);
		gp_display_spi_cmd(disp, ST7565_SETCOL_H);
		gp_display_spi_cmd(disp, ST7565_SETCOL_L);

		for (y = 0; y < 128; y++) {
			uint8_t byte = self->pixmap->pixels[p + y * 8];

			gp_display_spi_data(disp, byte);
		}
	}
}

static void st7565_repaint_part(gp_backend *self, gp_coord x0, gp_coord y0, gp_coord x1, gp_coord y1)
{
	//TODO: is partial repaint supported?
	st7565_repaint_full(self);
}

gp_backend *gp_st7565_init(const char *conn_id, unsigned int dpi)
{
	gp_backend *backend;
	struct gp_display_conn *conn = gp_display_conn_by_name(conn_id);
	int ret;

	backend = malloc(sizeof(gp_backend) + sizeof(struct gp_display_spi));
	if (!backend) {
		GP_WARN("malloc() failed :(");
		return NULL;
	}

	memset(backend, 0, sizeof(gp_backend) + sizeof(struct gp_display_spi));

	backend->pixmap = gp_pixmap_alloc(64, 128, GP_PIXEL_G1_DB);
	if (!backend->pixmap)
		goto err0;

	struct gp_display_spi *disp = GP_BACKEND_PRIV(backend);

	ret = gp_display_spi_init(disp, conn, SPI_MODE_3, 1000000, 64, 128);
	if (ret)
		goto err1;

	st7565_init(disp);

	backend->update = st7565_repaint_full;
	backend->update_rect = st7565_repaint_part;
	backend->exit = st7565_exit;

	backend->dpi = dpi;

	return backend;
err1:
	gp_pixmap_free(backend->pixmap);
err0:
	free(backend);
	return NULL;
}
