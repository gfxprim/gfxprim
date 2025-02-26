// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2025 Cyril Hrubis <metan@ucw.cz>
 */

#include <backends/gp_backend.h>
#include <core/gp_pixmap.h>
#include <core/gp_debug.h>

#include "gp_display_eink.h"
#include "gp_display_waveshare.h"

#include "gp_display_st77xx.h"

/*
 * ST7789 LCD driver constants
 */
enum st77xx_cmds {
	ST77XX_RESET = 0x01,

	ST77XX_SLEEP_IN = 0x10,
	ST77XX_SLEEP_OUT = 0x11,

	/**
	 * Normal mode on.
	 */
	ST77XX_NORM_ON = 0x13,

	ST77XX_INV_OFF = 0x20,
	ST77XX_INV_ON = 0x21,

	ST77XX_DISP_OFF = 0x28,
	ST77XX_DISP_ON = 0x29,

	/**
	 * Set column address
	 */
	ST77XX_COL_ADDR_SET = 0x2a,

	/**
	 * Sets row address
	 */
	ST77XX_ROW_ADDR_SET = 0x2b,

	/**
	 * Write RAM data.
	 */
	ST77XX_RAM_WRITE = 0x2c,


	/**
	 * Memory address control.
	 */
	ST77XX_MADDR_CTRL = 0x36,

	/**
	 * Sets pixel type:
	 *
	 * upper four bits
	 * 0101 - 65k colors
	 * 0110 - 262k colors
	 *
	 * lower four bits
	 * 0011 - 12 bpp
	 * 0101 - 16 bpp
	 * 0110 - 18 bpp
	 * 0111 - 16M truncated
	 *
	 * 0x55 == RGB565
         */
	ST77XX_COLOR_MODE = 0x3a,


	/**
	 * RAM control.
	 */
	ST77XX_RAM_CTRL = 0xb0,
};

static void st77xx_exit(gp_backend *self)
{
	struct gp_display_spi *disp = GP_BACKEND_PRIV(self);

	gp_display_spi_cmd(disp, ST77XX_DISP_OFF);
	gp_display_spi_exit(disp);

	free(self->pixmap);
	free(self);
}

static void st77xx_set_window(struct gp_display_spi *disp,
                              unsigned int min_x, unsigned int max_x,
			      unsigned int min_y, unsigned int max_y)
{
	min_x += disp->x_off;
	max_x += disp->x_off;

	min_y += disp->y_off;
	max_y += disp->y_off;

	gp_display_spi_cmd(disp, ST77XX_COL_ADDR_SET);

	gp_display_spi_data(disp, (min_x>>8) & 0xff);
	gp_display_spi_data(disp, (min_x) & 0xff);
	gp_display_spi_data(disp, (max_x>>8) & 0xff);
	gp_display_spi_data(disp, (max_x) & 0xff);

	gp_display_spi_cmd(disp, ST77XX_ROW_ADDR_SET);

	gp_display_spi_data(disp, (min_y>>8) & 0xff);
	gp_display_spi_data(disp, (min_y) & 0xff);
	gp_display_spi_data(disp, (max_y>>8) & 0xff);
	gp_display_spi_data(disp, (max_y) & 0xff);
}

static void st77xx_init(struct gp_display_spi *disp,
                        enum gp_display_st77xx_flags flags)
{
	/* Backlight */
	gp_gpio_write(&disp->gpio_map->pwr, 1);

	/* Reset display */
	gp_gpio_write(&disp->gpio_map->reset, 0);
	usleep(1000);
	gp_gpio_write(&disp->gpio_map->reset, 1);
	usleep(120000);

	gp_display_spi_cmd(disp, ST77XX_RESET);
	usleep(150000);
	gp_display_spi_cmd(disp, ST77XX_SLEEP_OUT);
	usleep(50000);

	gp_display_spi_cmd(disp, ST77XX_COLOR_MODE);
	gp_display_spi_data(disp, 0x55);
	usleep(10000);

	gp_display_spi_cmd(disp, ST77XX_MADDR_CTRL);
	gp_display_spi_data(disp, 0x00);

	if (flags & GP_DISPLAY_ST77XX_INV) {
		gp_display_spi_cmd(disp, ST77XX_INV_ON);
		usleep(10000);
	}

	gp_display_spi_cmd(disp, ST77XX_RAM_CTRL);
	gp_display_spi_data(disp, 0x00);
#if __BYTE_ORDER == __LITTLE_ENDIAN
	gp_display_spi_data(disp, 0xf8);
#else
	gp_display_spi_data(disp, 0xf0);
#endif

	gp_display_spi_cmd(disp, ST77XX_NORM_ON);
	usleep(10000);

	st77xx_set_window(disp, 0, disp->w-1, 0, disp->h-1);

	gp_display_spi_cmd(disp, ST77XX_DISP_ON);
	usleep(100000);
}

static void st77xx_repaint_full(gp_backend *self)
{
	struct gp_display_spi *disp = GP_BACKEND_PRIV(self);
	unsigned int i;

	/** Hack turn of display refresh when we write to the RAM */
	gp_display_spi_cmd(disp, ST77XX_RAM_CTRL);
	gp_display_spi_data(disp, 0x01);

	st77xx_set_window(disp, 0, disp->w-1, 0, disp->h-1);
	gp_display_spi_cmd(disp, ST77XX_RAM_WRITE);

	for (i = 0; i < disp->h/2; i++) {
		uint8_t *row = self->pixmap->pixels + i * self->pixmap->bytes_per_row*2;
		gp_display_spi_data_transfer(disp, row, NULL, self->pixmap->bytes_per_row*2);
	}

	gp_display_spi_cmd(disp, ST77XX_RAM_CTRL);
	gp_display_spi_data(disp, 0x00);
}

static enum gp_backend_ret st77xx_set_backlight(gp_backend *self,
                                                enum gp_backend_backlight_req backlight_req)
{
	struct gp_display_spi *disp = GP_BACKEND_PRIV(self);

	switch (backlight_req) {
	case GP_BACKEND_BACKLIGHT_INC:
		gp_gpio_write(&disp->gpio_map->pwr, 1);
		return 100;
	case GP_BACKEND_BACKLIGHT_DEC:
		gp_gpio_write(&disp->gpio_map->pwr, 0);
		return 0;
	case GP_BACKEND_BACKLIGHT_GET:
		//TODO
		return 0;
	}

	return GP_BACKEND_NOTSUPP;
}

static enum gp_backend_ret st77xx_set_attr(gp_backend *self,
                                           enum gp_backend_attr attr,
                                           const void *vals)
{
	switch (attr) {
	case GP_BACKEND_ATTR_FULLSCREEN:
	case GP_BACKEND_ATTR_TITLE:
	case GP_BACKEND_ATTR_SIZE:
	case GP_BACKEND_ATTR_CURSOR:
		return GP_BACKEND_NOTSUPP;
	case GP_BACKEND_ATTR_BACKLIGHT:
		return st77xx_set_backlight(self, *(enum gp_backend_backlight_req *)vals);
	}

	GP_WARN("Unsupported backend attribute %i", (int) attr);
	return GP_BACKEND_NOTSUPP;
}

static void st77xx_repaint_part(gp_backend *self,
                                gp_coord x0, gp_coord y0,
                                gp_coord x1, gp_coord y1)
{
	struct gp_display_spi *disp = GP_BACKEND_PRIV(self);
	gp_size w = x1-x0+1;
	gp_size h = y1-y0+1;
	gp_size bytes = w * h * 2;
	gp_size j;
	gp_coord y;

	gp_display_spi_cmd(disp, ST77XX_RAM_CTRL);
	gp_display_spi_data(disp, 0x01);

	st77xx_set_window(disp, x0, x1, y0, y1);
	gp_display_spi_cmd(disp, ST77XX_RAM_WRITE);

	/* Optimize small repaints into single write. */
	if (bytes <= 4096) {
		uint8_t buf[bytes];
		unsigned int pos = 0;

		for (y = y0; y <= y1; y++) {
			uint8_t *row = GP_PIXEL_ADDR(self->pixmap, x0, y);

			for (j = 0; j < w; j++) {
				buf[2*pos] = row[2*j];
				buf[2*pos+1] = row[2*j+1];
				pos++;
			}
		}

		gp_display_spi_data_transfer(disp, buf, NULL, bytes);
	} else {
		for (y = y0; y <= y1; y++) {
			uint8_t *row = GP_PIXEL_ADDR(self->pixmap, x0, y);
			gp_display_spi_data_transfer(disp, row, NULL, 2*w);
		}
	}

	gp_display_spi_cmd(disp, ST77XX_RAM_CTRL);
	gp_display_spi_data(disp, 0x00);
}

gp_backend *gp_display_st77xx_init(uint16_t w, uint16_t h, uint16_t x_off, uint16_t y_off,
                                   unsigned int dpi, enum gp_display_st77xx_flags flags)
{
	gp_backend *backend;
	int ret;

	backend = malloc(sizeof(gp_backend) + sizeof(struct gp_display_spi));
	if (!backend) {
		GP_WARN("malloc() failed :(");
		return NULL;
	}

	memset(backend, 0, sizeof(gp_backend) + sizeof(struct gp_display_spi));

	backend->pixmap = gp_pixmap_alloc(w, h, GP_PIXEL_RGB565);
	if (!backend->pixmap)
		goto err0;

	struct gp_display_spi *disp = GP_BACKEND_PRIV(backend);

	ret = gp_display_spi_init(disp, EINK_SPI_DEV, SPI_MODE_3,
	                          32000000, &gpio_map_rpi, w, h);
	if (ret)
		goto err1;

	disp->x_off = x_off;
	disp->y_off = y_off;

	st77xx_init(disp, flags);

	backend->flip = st77xx_repaint_full;
	backend->update_rect = st77xx_repaint_part;
	backend->set_attr = st77xx_set_attr;
	backend->exit = st77xx_exit;
	backend->dpi = dpi;

	return backend;
err1:
	gp_pixmap_free(backend->pixmap);
err0:
	free(backend);
	return NULL;
}
