// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2025 Cyril Hrubis <metan@ucw.cz>
 */

#include <backends/gp_backend.h>
#include <core/gp_pixmap.h>
#include <core/gp_debug.h>

#include "gp_display_eink.h"
#include "gp_display_waveshare.h"

#include "gp_display_st75256.h"

static void st75256_exit(gp_backend *self)
{
	struct gp_display_spi *disp = GP_BACKEND_PRIV(self);

	gp_display_spi_cmd(disp, ST75256_EXT1);
	gp_display_spi_cmd(disp, ST75256_DISP_OFF);
	gp_display_spi_exit(disp);

	free(self->pixmap);
	free(self);
}

/**
 * @param min_x Offset in pixels divided by (8/bpp) 0x00 - 0x28.
 * @param max_x Offset in pixels divided by (8/bpp) min_x - 0x28.
 * @param min_y Offset in pixels 0x00 - 0xff.
 * @param max_y Offset in pixels min_y - 0xff.
 */
static void sel_disp_range(struct gp_display_spi *self,
                           uint8_t min_x, uint8_t max_x,
			   uint8_t min_y, uint8_t max_y)
{
	gp_display_spi_cmd(self, ST75256_SET_COL_ADDR);
	gp_display_spi_data(self, min_y);
	gp_display_spi_data(self, max_y);

	gp_display_spi_cmd(self, ST75256_SET_PAGE_ADDR);
	gp_display_spi_data(self, min_x);
	gp_display_spi_data(self, max_x);
}

/**
 * Sets dark and light gray levels, value is between 0x00 and 0x1f
 */
static void set_gray_level(struct gp_display_spi *self,
                           uint8_t dark_gray, uint8_t light_gray)
{
	gp_display_spi_cmd(self, ST75256_EXT2);
	gp_display_spi_cmd(self, ST75256_SET_GRAY_LEVEL);

	gp_display_spi_data(self, 0x00);
	gp_display_spi_data(self, 0x00);
	gp_display_spi_data(self, 0x00);

	gp_display_spi_data(self, light_gray);
	gp_display_spi_data(self, light_gray);
	gp_display_spi_data(self, light_gray);

	gp_display_spi_data(self, 0x00);
	gp_display_spi_data(self, 0x00);

	gp_display_spi_data(self, dark_gray);

	gp_display_spi_data(self, 0x00);
	gp_display_spi_data(self, 0x00);

	gp_display_spi_data(self, dark_gray);
	gp_display_spi_data(self, dark_gray);
	gp_display_spi_data(self, dark_gray);

	gp_display_spi_data(self, 0x00);
	gp_display_spi_data(self, 0x00);

	gp_display_spi_cmd(self, ST75256_EXT1);
}

static void read_otp(struct gp_display_spi *self)
{
	gp_display_spi_cmd(self, ST75256_EXT2);

	/* Disable OTP auto read */
	gp_display_spi_cmd_data(self, ST75256_OTP_AUTO_READ_CTRL, 0x9f);

	/* Enable OTP read */
	gp_display_spi_cmd_data(self, ST75256_OTP_WR_RD_CTRL, 0x00);
	usleep(10000);

	/* Load OTP */
	gp_display_spi_cmd(self, ST75256_OTP_READ);
	usleep(20000);

	/* Exit OTP */
	gp_display_spi_cmd(self, ST75256_OTP_CTRL_OUT);
}

static void st75256_init(struct gp_display_spi *disp, unsigned int bpp)
{
	/* Reset display */
	gp_gpio_write(&disp->gpio_map->reset, 0);
	usleep(1000);
	gp_gpio_write(&disp->gpio_map->reset, 1);
	usleep(120000);

	/* Wake up the display */
	gp_display_spi_cmd(disp, ST75256_EXT1);
	gp_display_spi_cmd(disp, ST75256_SLEEP_OUT);
	usleep(50000);
	gp_display_spi_cmd(disp, ST75256_DISP_OFF);

	read_otp(disp);

	gp_display_spi_cmd(disp, ST75256_EXT1);

	/* Set column page direction - page direction first & swap X */
	gp_display_spi_cmd_data(disp, ST75256_SET_SCAN_DIRECTION, 0x06);

	/* Inverse display so that 00 is black */
	gp_display_spi_cmd(disp, ST75256_DISP_INV);

	/* Turn on internal voltage generators */
	gp_display_spi_cmd_data(disp, ST75256_PWR_CTRL, 0x0b);
	usleep(1000);

	/* Sets contrast */
	gp_display_spi_cmd(disp, ST75256_SET_VOP);
	gp_display_spi_data(disp, 0x38);
	gp_display_spi_data(disp, 0x04);

	/* Set gray levels */
	set_gray_level(disp, 0x0b, 0x0f);

/*
	gp_display_spi_cmd(disp, ST75256_ANALOG_CIRCUIT_SET);
	gp_display_spi_data(disp, 0x00);
	gp_display_spi_data(disp, 0x01);
	gp_display_spi_data(disp, 0x02);
*/
	/* Set 1bpp or 2bpp mode */
	gp_display_spi_cmd_data(disp, ST75256_DISP_MODE, bpp == 1 ? 0x10 : 0x11);

	gp_display_spi_cmd(disp, ST75256_DISP_CTRL);
	gp_display_spi_data(disp, 0x00);
	gp_display_spi_data(disp, 0x7f);
	gp_display_spi_data(disp, 0x00);

	gp_display_spi_cmd(disp, ST75256_DISP_ON);
}

static void st75256_2bpp_repaint_full(gp_backend *self)
{
	struct gp_display_spi *disp = GP_BACKEND_PRIV(self);
	unsigned int y;

	sel_disp_range(disp, 0, disp->w/4-1, 0, disp->h-1);
	gp_display_spi_cmd(disp, ST75256_WRITE_DATA);

	for (y = 0; y < 256; y++) {
		uint8_t *row = &(self->pixmap->pixels[32 * y]);
		gp_display_spi_data_transfer(disp, row, NULL, 32);
	}
}

static void st75256_1bpp_repaint_full(gp_backend *self)
{
	struct gp_display_spi *disp = GP_BACKEND_PRIV(self);
	unsigned int y;

	sel_disp_range(disp, 0, disp->w/8-1, 0, disp->h-1);
	gp_display_spi_cmd(disp, ST75256_WRITE_DATA);

	for (y = 0; y < 256; y++) {
		uint8_t *row = &(self->pixmap->pixels[16 * y]);
		gp_display_spi_data_transfer(disp, row, NULL, 16);
	}
}

static void st75256_2bpp_repaint_part(gp_backend *self,
                                      gp_coord x0, gp_coord y0,
                                      gp_coord x1, gp_coord y1)
{
	struct gp_display_spi *disp = GP_BACKEND_PRIV(self);
	/* round the x coordinates to whole bytes */
	unsigned int min_x = (x0)/4;
	unsigned int max_x = (x1+3)/4;
	unsigned int width = max_x - min_x + 1;
	int y;

	sel_disp_range(disp, min_x, max_x, y0, y1);
	gp_display_spi_cmd(disp, ST75256_WRITE_DATA);

	for (y = y0; y <= y1; y++) {
		uint8_t *row = &(self->pixmap->pixels[32 * y + min_x]);
		gp_display_spi_data_transfer(disp, row, NULL, width);
	}
}

static void st75256_1bpp_repaint_part(gp_backend *self,
                                      gp_coord x0, gp_coord y0,
                                      gp_coord x1, gp_coord y1)
{
	struct gp_display_spi *disp = GP_BACKEND_PRIV(self);
	/* round the x coordinates to whole bytes */
	unsigned int min_x = x0/8;
	unsigned int max_x = (x1+7)/8;
	unsigned int width = max_x - min_x + 1;
	int y;

	sel_disp_range(disp, min_x, max_x, y0, y1);
	gp_display_spi_cmd(disp, ST75256_WRITE_DATA);

	for (y = y0; y <= y1; y++) {
		uint8_t *row = &(self->pixmap->pixels[16 * y + min_x]);
		gp_display_spi_data_transfer(disp, row, NULL, width);
	}
}

gp_backend *gp_st75256_init(unsigned int dpi, unsigned int bpp)
{
	gp_backend *backend;
	int ret;

	switch (bpp) {
	case 1:
	case 2:
		break;
	default:
		GP_FATAL("Invalid bpp %u\n", bpp);
		return NULL;
	};

	backend = malloc(sizeof(gp_backend) + sizeof(struct gp_display_spi));
	if (!backend) {
		GP_WARN("malloc() failed :(");
		return NULL;
	}

	memset(backend, 0, sizeof(gp_backend) + sizeof(struct gp_display_spi));

	backend->pixmap = gp_pixmap_alloc(128, 256, bpp == 1 ? GP_PIXEL_G1_UB : GP_PIXEL_G2_UB);
	if (!backend->pixmap)
		goto err0;

	struct gp_display_spi *disp = GP_BACKEND_PRIV(backend);

	ret = gp_display_spi_init(disp, EINK_SPI_DEV, SPI_MODE_0,
	                          12500000, &gpio_map_rpi, 128, 256);
	if (ret)
		goto err1;

	st75256_init(disp, bpp);

	switch (bpp) {
	case 1:
		backend->flip = st75256_1bpp_repaint_full;
		backend->update_rect = st75256_1bpp_repaint_part;
	break;
	case 2:
		backend->flip = st75256_2bpp_repaint_full;
		backend->update_rect = st75256_2bpp_repaint_part;
	break;
	}

	backend->exit = st75256_exit;
	backend->dpi = dpi;

	return backend;
err1:
	gp_pixmap_free(backend->pixmap);
err0:
	free(backend);
	return NULL;
}
