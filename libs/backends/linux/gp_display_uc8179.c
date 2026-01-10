// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2023 Cyril Hrubis <metan@ucw.cz>
 */

#include <inttypes.h>
#include <backends/gp_backend.h>
#include <core/gp_pixmap.h>
#include <core/gp_debug.h>

#include "gp_display_eink.h"
#include "gp_display_conn.h"
#include "gp_display_uc8179.h"
#include "gp_display_uc8179_luts.h"

static void uc81xx_tres(struct gp_display_spi *disp, uint16_t w, uint16_t h)
{
	uint8_t tx_buf[4] = {w>>8, w & 0xff, h>>8, h & 0xff};

	GP_DEBUG(4, "Setting display resolution %"PRIu16" x %"PRIu16, w, h);

	gp_display_spi_cmd(disp, UC8179_TRES);
	gp_display_spi_data_write(disp, tx_buf, sizeof(tx_buf));
}

static void uc81xx_init(struct gp_display_spi *self)
{
	GP_DEBUG(4, "Turning on display power & hardware reset");

	gp_gpio_write(&self->gpio_map->reset, 0);
	usleep(1000);
	gp_gpio_write(&self->gpio_map->pwr, 1);
	usleep(10000);
	gp_gpio_write(&self->gpio_map->reset, 1);
	usleep(10000);

	/* Set resolution */
	uc81xx_tres(self, self->w, self->h);

	/*
	 * Set border to Hi-Z so that it does not flash on refresh
	 *
	 * And LUT for BW so that 1 == white and 0 == black
	 */
	gp_display_spi_cmd(self, UC8179_VCON);
	gp_display_spi_data(self, 0x8b);
}

static void uc81xx_deep_sleep(struct gp_display_spi *self)
{
	gp_display_spi_cmd(self, UC8179_DSLP);
}

static void display_exit(gp_backend *self)
{
	struct gp_display_eink *eink = GP_BACKEND_PRIV(self);

	uc81xx_deep_sleep(&eink->spi);

	gp_display_spi_exit(&eink->spi);
	free(self);
}

static void repaint_full_start(gp_backend *self)
{
	struct gp_display_eink *eink = GP_BACKEND_PRIV(self);
	struct gp_display_spi *disp = &eink->spi;

	unsigned int y;

	/* Power on and wait for ready */
	gp_display_spi_cmd(disp, UC8179_PON);
	gp_display_spi_wait_ready(disp, 1);

	/* Start data transfer into RAM */
	gp_display_spi_cmd(disp, UC8179_DTM2);

	for (y = 0; y < 480; y++) {
		uint8_t *tx_buf = &self->pixmap->pixels[100 * y];

		gp_display_spi_data_write(disp, tx_buf, 100);
	}

	/* Setup interrupt source */
	gp_display_spi_busy_edge_set(disp, GP_GPIO_EDGE_RISE);

	uc8179_write_lut(disp, &gp_uc8179_bw_lut_full);

	/* LUT from OTP, Black & White, buffer direction settings */
	gp_display_spi_cmd(disp, UC8179_PSR);
	gp_display_spi_data(disp, UC8179_PSR_KW | UC8179_PSR_LUT_REG |
	                          UC8179_PSR_UD | UC8179_PSR_SHL |
				  UC8179_PSR_SHD_N | UC8179_PSR_RST_N);

	/* Refresh display */
	gp_display_spi_cmd(disp, UC8179_DRF);
}

static void repaint_full_finish(gp_backend *self)
{
	struct gp_display_eink *eink = GP_BACKEND_PRIV(self);
	struct gp_display_spi *spi = &eink->spi;

	/* Disable interrupt source */
	gp_display_spi_busy_edge_set(spi, GP_GPIO_EDGE_NONE);

	/* Power off and wait for ready */
	gp_display_spi_cmd(spi, UC8179_POF);
	gp_display_spi_wait_ready(spi, 1);
}

static void repaint_part_start(gp_backend *self, gp_coord x0, gp_coord y0, gp_coord x1, gp_coord y1)
{
	struct gp_display_eink *eink = GP_BACKEND_PRIV(self);
	struct gp_display_spi *disp = &eink->spi;

	uint16_t horiz_start = x0 & ~0x07;
	uint16_t horiz_end = x1 | 0x07;
	uint16_t vert_start = y0;
	uint16_t vert_end = y1;

	/* Power on and wait for ready */
	gp_display_spi_cmd(disp, UC8179_PON);
	gp_display_spi_wait_ready(disp, 1);

	/* Set partial refresh window, 10bit integers horizontal direction is rounded to whole bytes */
	gp_display_spi_cmd(disp, UC8179_PTL);

	gp_display_spi_data(disp, horiz_start>>8);
	gp_display_spi_data(disp, horiz_start);
	gp_display_spi_data(disp, horiz_end>>8);
	gp_display_spi_data(disp, horiz_end&0xff);

	gp_display_spi_data(disp, vert_start>>8);
	gp_display_spi_data(disp, vert_start&0xff);
	gp_display_spi_data(disp, vert_end>>8);
	gp_display_spi_data(disp, vert_end&0xff);

	/* Enter partial mode */
	gp_display_spi_cmd(disp, UC8179_PTIN);

	/* Start partial data transfer into RAM */
	gp_display_spi_cmd(disp, UC8179_DTM2);

	gp_coord y;

	size_t len = (horiz_end - horiz_start)/8 + 1;

	for (y = y0; y <= y1; y++) {
		uint8_t *tx_buf = &self->pixmap->pixels[100 * y + x0/8];

		gp_display_spi_data_write(disp, tx_buf, len);
	}

	/* Exit partial mode */
	gp_display_spi_cmd(disp, UC8179_PTOUT);

	/* Setup interrupt source */
	gp_display_spi_busy_edge_set(disp, GP_GPIO_EDGE_RISE);

	uc8179_write_lut(disp, &gp_uc8179_bw_lut_part);

	/* LUT from Register, Black & White, buffer direction settings */
	gp_display_spi_cmd(disp, UC8179_PSR);
	gp_display_spi_data(disp, UC8179_PSR_KW | UC8179_PSR_LUT_REG |
	                          UC8179_PSR_UD | UC8179_PSR_SHL |
				  UC8179_PSR_SHD_N | UC8179_PSR_RST_N);

	/* Refresh display */
	gp_display_spi_cmd(disp, UC8179_DRF);
}

static void repaint_part_finish(gp_backend *self)
{
	struct gp_display_eink *eink = GP_BACKEND_PRIV(self);
	struct gp_display_spi *spi = &eink->spi;

	/* Disable interrupt source */
	gp_display_spi_busy_edge_set(spi, GP_GPIO_EDGE_NONE);

	/* Power off and wait for ready */
	gp_display_spi_cmd(spi, UC8179_POF);
	gp_display_spi_wait_ready(spi, 1);
}

gp_backend *gp_waveshare_7_5_v2_init(const char *conn_id)
{
	struct gp_display_conn *conn = gp_display_conn_by_name(conn_id);
	gp_backend *backend;
	int ret;

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

	ret = gp_display_spi_init(&eink->spi, conn, EINK_SPI_MODE, EINK_SPI_SPEED_HZ, 800, 480);
	if (ret)
		goto err1;

	uc81xx_init(&eink->spi);

	eink->full_repaint_ms = 2000;
	eink->part_repaint_ms = 1000;
	eink->repaint_full_start = repaint_full_start;
	eink->repaint_full_finish = repaint_full_finish;
	eink->repaint_part_start = repaint_part_start;
	eink->repaint_part_finish = repaint_part_finish;
	eink->display_exit = display_exit;

	gp_display_eink_init(backend);

	backend->dpi = 125;

	return backend;
err1:
	gp_pixmap_free(backend->pixmap);
err0:
	free(backend);
	return NULL;
}
