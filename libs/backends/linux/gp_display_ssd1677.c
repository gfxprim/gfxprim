// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2023 Cyril Hrubis <metan@ucw.cz>
 */

#include <backends/gp_backend.h>
#include <core/gp_pixmap.h>
#include <core/gp_debug.h>

#include "gp_display_eink.h"
#include "gp_display_waveshare.h"
#include "gp_display_ssd1677.h"

static const uint8_t lut_1bpp_DU[SSD1677_LUT_SIZE] =
{
    /* Source and VCOM voltages */
    0x2A,0x05,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//1
    0x05,0x2A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//2
    0x2A,0x15,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//3
    0x05,0x0A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//4
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//5
    /* 4 x Phase lenght + 1 x Repeat */
    0x00,0x02,0x03,0x0A,0x00,
    0x02,0x06,0x0A,0x05,0x00,
    0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,
    /* Frame rate */
    0x22,0x22,0x22,0x22,0x22
};

static const uint8_t lut_1bpp_A2[SSD1677_LUT_SIZE] =
{
    /* Source and VCOM voltages */
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //1
    0x0A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //2
    0x05,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //3
    0x0A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //4
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //5
    /* 4 x Phase lenght + 1 x Repeat */
    0x00,0x00,0x03,0x05,0x00,
    0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,
    /* Frame rate */
    0x22,0x22,0x22,0x22,0x22
};

static void load_lut(struct gp_display_spi *self, const uint8_t *lut)
{
	GP_DEBUG(4, "Loading LUT");
	gp_display_spi_cmd(self, 0x32);
	gp_display_spi_data_transfer(self, lut, NULL, SSD1677_LUT_SIZE);
}

static void ssd1677_set_ram_addr(struct gp_display_spi *spi, uint16_t x, uint16_t y)
{
	GP_DEBUG(3, "Setting RAM start %u x %u", x, y);

	gp_display_spi_cmd(spi, SSD1677_XRAM_ADDR);
	gp_display_spi_data(spi, x & 0xff);
	gp_display_spi_data(spi, (x >> 8) & 0x03);

	gp_display_spi_cmd(spi, SSD1677_YRAM_ADDR);
	gp_display_spi_data(spi, y & 0xff);
	gp_display_spi_data(spi, (y >> 8) & 0x03);
}

static void ssd1677_set_ram_window(struct gp_display_spi *spi,
                                   uint16_t x1, uint16_t x2,
                                   uint16_t y1, uint16_t y2)
{
	GP_DEBUG(3, "Setting RAM window %u-%u x %u-%u", x1, x2, y1, y2);

	gp_display_spi_cmd(spi, SSD1677_XRAM_RANGE);
	gp_display_spi_data(spi, x1 & 0xff);
	gp_display_spi_data(spi, (x1 >> 8) & 0x03);
	gp_display_spi_data(spi, x2 & 0xff);
	gp_display_spi_data(spi, (x2 >> 8) & 0x03);

	gp_display_spi_cmd(spi, SSD1677_YRAM_RANGE);
	gp_display_spi_data(spi, y1 & 0xff);
	gp_display_spi_data(spi, (y1 >> 8) & 0x03);
	gp_display_spi_data(spi, y2 & 0xff);
	gp_display_spi_data(spi, (y2 >> 8) & 0x03);
}

static void ssd1677_reset_ram_window(struct gp_display_spi *spi)
{
	ssd1677_set_ram_window(spi, 0, spi->w-1, 0, spi->h-1);
}

static void ssd1677_clear_red_ram(struct gp_display_spi *spi)
{
	gp_display_spi_cmd(spi, SSD1677_CLR_RED_RAM);
	gp_display_spi_data(spi, 0xf7);
	gp_display_spi_wait_ready(spi, 0);
}

static void ssd1677_clear_bw_ram(struct gp_display_spi *spi)
{
	gp_display_spi_cmd(spi, SSD1677_CLR_BW_RAM);
	gp_display_spi_data(spi, 0xf7);
	gp_display_spi_wait_ready(spi, 0);
}

static void waveshare_3_7_init(struct gp_display_spi *self)
{
	GP_DEBUG(4, "Turning on display power & hardware reset");

	gp_gpio_write(&self->gpio_map->reset, 0);
	usleep(1000);
	gp_gpio_write(&self->gpio_map->pwr, 1);
	usleep(10000);
	gp_gpio_write(&self->gpio_map->reset, 1);
	usleep(10000);

	gp_display_spi_cmd(self, SSD1677_SW_RESET);
	usleep(10000);

	ssd1677_clear_red_ram(self);
	ssd1677_clear_bw_ram(self);

	gp_display_spi_cmd(self, SSD1677_DOC);
	gp_display_spi_data(self, 0xdf);
	gp_display_spi_data(self, 0x01);
	gp_display_spi_data(self, 0x00);

	gp_display_spi_cmd(self, SSD1677_BSST);
	gp_display_spi_data(self, 0xae);
	gp_display_spi_data(self, 0xc7);
	gp_display_spi_data(self, 0xc3);
	gp_display_spi_data(self, 0xc0);
	gp_display_spi_data(self, 0xc0);

	gp_display_spi_cmd(self, SSD1677_TEMP_CTRL);
	gp_display_spi_data(self, 0x80);

	/* Display Mode */
	gp_display_spi_cmd(self, SSD1677_WRDO);
	gp_display_spi_data(self, 0x00);
	gp_display_spi_data(self, 0xff);
	gp_display_spi_data(self, 0xff);
	gp_display_spi_data(self, 0xff);
	gp_display_spi_data(self, 0xff);
	gp_display_spi_data(self, 0x4f);
	gp_display_spi_data(self, 0xff);
	gp_display_spi_data(self, 0xff);
	gp_display_spi_data(self, 0xff);
	gp_display_spi_data(self, 0xff);

	ssd1677_reset_ram_window(self);

	gp_display_spi_cmd(self, SSD1677_UPDT_CTRL2);
	gp_display_spi_data(self, 0xc4);
}

static void ssd1677_deep_sleep(struct gp_display_spi *self)
{
	GP_DEBUG(4, "Entering deep sleep");
	gp_display_spi_cmd(self, SSD1677_DSLP);
	gp_display_spi_cmd(self, 0x03);
	usleep(10000);
}

static void display_exit(gp_backend *self)
{
	struct gp_display_eink *eink = GP_BACKEND_PRIV(self);

	ssd1677_deep_sleep(&eink->spi);
	gp_display_spi_exit(&eink->spi);
	free(self);
}

static void repaint_full_start(gp_backend *self)
{
	struct gp_display_eink *eink = GP_BACKEND_PRIV(self);
	struct gp_display_spi *spi = &eink->spi;
	unsigned int y;

	ssd1677_reset_ram_window(spi);

	ssd1677_set_ram_addr(spi, 0, 0);

	gp_display_spi_cmd(spi, SSD1677_WRITE_BW_RAM);

	uint16_t line_bytes = spi->w/8;

	for (y = 0; y < spi->h; y++) {
		uint8_t *tx_buf = &self->pixmap->pixels[line_bytes * y];

		gp_display_spi_data_transfer(spi, tx_buf, NULL, line_bytes);
	}

	load_lut(spi, lut_1bpp_DU);
	gp_display_spi_busy_edge_set(spi, GP_GPIO_EDGE_FALL);
	gp_display_spi_cmd(spi, SSD1677_DISP_UPDT);
}

static void repaint_full_finish(gp_backend *self)
{
	struct gp_display_eink *eink = GP_BACKEND_PRIV(self);
	struct gp_display_spi *spi = &eink->spi;

	gp_display_spi_busy_edge_set(spi, GP_GPIO_EDGE_NONE);
}

static void repaint_part_start(gp_backend *self,
                               gp_coord x0, gp_coord y0,
                               gp_coord x1, gp_coord y1)
{
	struct gp_display_eink *eink = GP_BACKEND_PRIV(self);
	struct gp_display_spi *spi = &eink->spi;

	uint16_t x_start = x0 & ~0x07;
	uint16_t x_end = (x1 + 0x06) & ~0x07;
	uint16_t y_start = y0;
	uint16_t y_end = y1;
	unsigned int y;

	ssd1677_set_ram_window(spi, x_start, x_end, y_start, y_end);
	ssd1677_set_ram_addr(spi, x_start, y_start);

	gp_display_spi_cmd(spi, SSD1677_WRITE_BW_RAM);

	unsigned int line_bytes = spi->w/8;
	unsigned int len = (x_end - x_start)/8  + 1;

	for (y = y_start; y <= y_end; y++) {
		uint8_t *tx_buf = &self->pixmap->pixels[line_bytes * y + x_start/8];

		gp_display_spi_data_transfer(spi, tx_buf, NULL, len);
	}

	load_lut(spi, lut_1bpp_A2);
	gp_display_spi_busy_edge_set(spi, GP_GPIO_EDGE_FALL);
	gp_display_spi_cmd(spi, SSD1677_DISP_UPDT);
}

static void repaint_part_finish(gp_backend *self)
{
	struct gp_display_eink *eink = GP_BACKEND_PRIV(self);
	struct gp_display_spi *spi = &eink->spi;

	gp_display_spi_busy_edge_set(spi, GP_GPIO_EDGE_NONE);
}

gp_backend *gp_waveshare_3_7_init(void)
{
	int ret;
	gp_backend *backend;

	backend = malloc(sizeof(gp_backend) + sizeof(struct gp_display_eink));
	if (!backend) {
		GP_WARN("malloc() failed :(");
		return NULL;
	}

	memset(backend, 0, sizeof(gp_backend) + sizeof(struct gp_display_eink));

	backend->pixmap = gp_pixmap_alloc(280, 480, GP_PIXEL_G1);
	if (!backend->pixmap)
		goto err0;

	struct gp_display_eink *eink = GP_BACKEND_PRIV(backend);

	ret = gp_display_spi_init(&eink->spi, EINK_SPI_DEV, EINK_SPI_MODE, EINK_SPI_SPEED_HZ, &gpio_map_rpi, 280, 480);
	if (ret)
		goto err1;

	waveshare_3_7_init(&eink->spi);

	eink->full_repaint_ms = 1000;
	eink->part_repaint_ms = 500;
	eink->repaint_full_start = repaint_full_start;
	eink->repaint_full_finish = repaint_full_finish;
	eink->repaint_part_start = repaint_part_start;
	eink->repaint_part_finish = repaint_part_finish;
	eink->display_exit = display_exit;

	gp_display_eink_init(backend);

	backend->dpi = 150;

	return backend;
err1:
	gp_pixmap_free(backend->pixmap);
err0:
	free(backend);
	return NULL;
}
