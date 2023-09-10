// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2023 Cyril Hrubis <metan@ucw.cz>
 */

#include <backends/gp_backend.h>
#include <core/gp_pixmap.h>
#include <core/gp_debug.h>

#include "gp_display_eink.h"
#include "gp_display_waveshare.h"
#include "gp_display_spd1656.h"

static void eink_hw_init(struct gp_display_spi *self)
{
	GP_DEBUG(4, "Turning on display power & hardware reset");

	gp_gpio_write(&self->gpio_map->reset, 0);
	usleep(1000);
	gp_gpio_write(&self->gpio_map->pwr, 1);
	usleep(10000);
	gp_gpio_write(&self->gpio_map->reset, 1);
	usleep(10000);

	/* Black & White */
	gp_display_spi_cmd(self, SPD1656_PSR);
	/* LUT from register, Black & White, buffer direction settings */
	gp_display_spi_data(self, 0x1f);

//	gp_display_spi_cmd(self, SPD1656_PWR);
	/* turn on VSR, VS, VG internal DC/DC */
//	gp_display_spi_data(self, 0x27);
	/* VGH VGL voltage level +20V -20V (default) */
//	gp_display_spi_data(self, 0x07);
	/* VDH level 15V (default 14V) */
//	gp_display_spi_data(self, 0x3f);
	/* VDL level -15V (default -14V) */
//	gp_display_spi_data(self, 0x3f);

	/* Sets VCOM DC voltage == contrast */
//	gp_display_spi_cmd(self, SPD1656_VDCS);
//	gp_display_spi_data(self, 0x24);

	/* Booster settings */
	//gp_display_spi_cmd(self, SPD1656_BSST);
	//gp_display_spi_data(self, 0x17);
	//gp_display_spi_data(self, 0x17);
	//gp_display_spi_data(self, 0x28);

	/* Set PLL to 100Hz */
//	gp_display_spi_cmd(self, SPD1656_PLL);
//	gp_display_spi_data(self, 0x0f);

	/* Set 800x480 resolution */
	gp_display_spi_cmd(self, SPD1656_TRES);
	gp_display_spi_data(self, 0x03);
	gp_display_spi_data(self, 0x20);
	gp_display_spi_data(self, 0x01);
	gp_display_spi_data(self, 0xe0);

	/*
	 * Set border to Hi-Z so that it does not flash on refresh
	 *
	 * And LUT for BW so that 1 == white and 0 == black
	 */
	gp_display_spi_cmd(self, SPD1656_VCON);
	gp_display_spi_data(self, 0x83);

//	gp_display_spi_cmd(self, SPD1656_TCON);
//	gp_display_spi_data(self, 0x77);

//	gp_display_spi_cmd(self, SPD1656_GSST);
//	gp_display_spi_data(self, 0x00);
//	gp_display_spi_data(self, 0x00);
//	gp_display_spi_data(self, 0x00);
//	gp_display_spi_data(self, 0x00);

	/* Fill OLD data with zeroes otherwise refresh will be messy */
	gp_display_spi_cmd(self, SPD1656_DTM1);
	unsigned int x, y;

	for (y = 0; y < 480; y++) {
		for (x = 0; x < 100; x++) {
			//TODO: write whole lines
			gp_display_spi_data(self, 0);
		}
	}
}

static void eink_hw_exit(struct gp_display_spi *self)
{
	gp_display_spi_cmd(self, SPD1656_POF);
	gp_display_spi_wait_ready(self, 1);
	gp_display_spi_cmd(self, SPD1656_DSLP);
	gp_display_spi_exit(self);
}

static void backend_exit(gp_backend *self)
{
	struct gp_display_eink *eink = GP_BACKEND_PRIV(self);

	eink_hw_exit(&eink->spi);

	free(self);
}

static void repaint_full_start(gp_backend *self)
{
	unsigned int x, y;
	struct gp_display_eink *eink = GP_BACKEND_PRIV(self);
	struct gp_display_spi *spi = &eink->spi;

	/* Power on and wait for ready */
	gp_display_spi_cmd(spi, SPD1656_PON);
	gp_display_spi_wait_ready(spi, 1);

	/* Start data transfer into RAM */
	gp_display_spi_cmd(spi, SPD1656_DTM2);

	for (y = 0; y < 480; y++) {
		for (x = 0; x < 100; x++) {
			uint8_t byte = self->pixmap->pixels[100 * y + x];
			//TODO: write whole lines
			gp_display_spi_data(spi, byte);
		}
	}

	/* Refresh display */
	gp_display_spi_cmd(spi, SPD1656_DRF);
}

static void repaint_full_finish(gp_backend *self)
{
	struct gp_display_eink *eink = GP_BACKEND_PRIV(self);
	struct gp_display_spi *spi = &eink->spi;

	gp_display_spi_wait_ready(spi, 1);

	/* Power off and wait for ready */
	gp_display_spi_cmd(spi, SPD1656_POF);
	gp_display_spi_wait_ready(spi, 1);
}

static void repaint_part_start(gp_backend *self, gp_coord x0, gp_coord y0, gp_coord x1, gp_coord y1)
{
	struct gp_display_eink *eink = GP_BACKEND_PRIV(self);
	struct gp_display_spi *spi = &eink->spi;

	uint16_t horiz_start = (x0 & ~0x07);
	uint16_t horiz_end = x1 & ~0x07;
	uint16_t vert_start = y0;
	uint16_t vert_end = y1;

	/* Power on and wait for ready */
	gp_display_spi_cmd(spi, SPD1656_PON);
	gp_display_spi_wait_ready(spi, 1);

	/* Set partial refresh window, 10bit integers horizontal direction is rounded to whole bytes */
	gp_display_spi_cmd(spi, SPD1656_PTL);

	gp_display_spi_data(spi, horiz_start>>8);
	gp_display_spi_data(spi, horiz_start);
	gp_display_spi_data(spi, horiz_end>>8);
	gp_display_spi_data(spi, (horiz_end&0xff) | 0x07);

	gp_display_spi_data(spi, vert_start>>8);
	gp_display_spi_data(spi, vert_start&0xff);
	gp_display_spi_data(spi, vert_end>>8);
	gp_display_spi_data(spi, vert_end&0xff);

	/* Enter partial mode */
	gp_display_spi_cmd(spi, SPD1656_PTIN);

	/* Start partial data transfer into RAM */
	gp_display_spi_cmd(spi, SPD1656_DTM2);

	uint16_t x, y;

	for (y = y0; y <= y1; y++) {
		for (x = x0/8; x <= x1/8; x++) {
			uint8_t byte = self->pixmap->pixels[100 * y + x];
			//TODO: write whole lines
			gp_display_spi_data(spi, byte);
		}
	}

	/* Refresh display */
	gp_display_spi_cmd(spi, SPD1656_DRF);
}

static void repaint_part_finish(gp_backend *self)
{
	struct gp_display_eink *eink = GP_BACKEND_PRIV(self);
	struct gp_display_spi *spi = &eink->spi;

	gp_display_spi_wait_ready(spi, 1);

	/* Exit partial mode */
	gp_display_spi_cmd(spi, SPD1656_PTOUT);

	/* Power off and wait for ready */
	gp_display_spi_cmd(spi, SPD1656_POF);
	gp_display_spi_wait_ready(spi, 1);
}

static int is_busy(gp_backend *self)
{
	struct gp_display_eink *eink = GP_BACKEND_PRIV(self);
	struct gp_display_spi *spi = &eink->spi;

	return gp_display_spi_busy(spi) == 0;
}

gp_backend *gp_waveshare_7_5_v2_init(void)
{
	int ret;
	gp_backend *backend;

	backend = malloc(sizeof(gp_backend) + sizeof(struct gp_display_eink));
	if (!backend) {
		GP_WARN("malloc() failed :(");
		return NULL;
	}

	memset(backend, 0, sizeof(gp_backend) + sizeof(struct gp_display_eink));

	backend->pixmap = gp_pixmap_alloc(800, 480, GP_PIXEL_G1);
	if (!backend->pixmap)
		goto err0;

	struct gp_display_eink *eink = GP_BACKEND_PRIV(backend);

	ret = gp_display_spi_init(&eink->spi, EINK_SPI_DEV, EINK_SPI_MODE, EINK_SPI_SPEED_HZ, &gpio_map_rpi);
	if (ret)
		goto err1;

	eink_hw_init(&eink->spi);

	eink->full_repaint_ms = 4000;
	eink->part_repaint_ms = 4000;
	eink->repaint_full_start = repaint_full_start;
	eink->repaint_full_finish = repaint_full_finish;
	eink->repaint_part_start = repaint_part_start;
	eink->repaint_part_finish = repaint_part_finish;
	eink->is_busy = is_busy;

	gp_display_eink_init(backend);

	backend->exit = backend_exit;
	backend->dpi = 125;

	return backend;
err1:
	gp_pixmap_free(backend->pixmap);
err0:
	free(backend);
	return NULL;
}
