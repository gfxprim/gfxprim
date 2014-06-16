/*****************************************************************************
 * This file is part of gfxprim library.                                     *
 *                                                                           *
 * Gfxprim is free software; you can redistribute it and/or                  *
 * modify it under the terms of the GNU Lesser General Public                *
 * License as published by the Free Software Foundation; either              *
 * version 2.1 of the License, or (at your option) any later version.        *
 *                                                                           *
 * Gfxprim is distributed in the hope that it will be useful,                *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Lesser General Public License for more details.                           *
 *                                                                           *
 * You should have received a copy of the GNU Lesser General Public          *
 * License along with gfxprim; if not, write to the Free Software            *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,                        *
 * Boston, MA  02110-1301  USA                                               *
 *                                                                           *
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include <string.h>
#include <errno.h>

#include <core/GP_Common.h>
#include <loaders/GP_IO.h>

#include "tst_test.h"

/*
 * Expects IO buffer filled with monotonically increasing bytes, i.e.
 * 0x00 0x01 0x02 ...
 */
static int do_test(GP_IO *io)
{
	int ret;
	uint8_t buf[10];
	unsigned int i;

	ret = GP_IORead(io, buf, 10);

	if (ret != 10) {
		tst_msg("First IO read failed");
		return TST_FAILED;
	}

	for (i = 0; i < 10; i++) {
		if (i != buf[i]) {
			tst_msg("Read wrong data at %u", i);
			return TST_FAILED;
		}
	}

	ret = GP_IOTell(io);

	if (ret != 10) {
		tst_msg("Have wrong offset %u, after read 10", ret);
		return TST_FAILED;
	}


	ret = GP_IORead(io, buf, 10);

	if (ret != 10) {
		tst_msg("Second IO read failed");
		return TST_FAILED;
	}

	for (i = 0; i < 10; i++) {
		if (i + 10 != buf[i]) {
			tst_msg("Read wrong data at %u", i + 10);
			return TST_FAILED;
		}
	}

	if (GP_IORewind(io)) {
		tst_msg("Failed to rewind to start");
		return TST_FAILED;
	}

	ret = GP_IOTell(io);

	if (ret != 0) {
		tst_msg("Have wrong offset %u, after rewind", ret);
		return TST_FAILED;
	}

	uint16_t header[] = {
		0x00,
		0x01,
		GP_IO_BYTE,
		GP_IO_IGN | 7,
		GP_IO_L2,
		GP_IO_END
	};

	uint8_t byte;
	uint16_t val;

	if (GP_IOReadF(io, header, &byte, &val) != 5) {
		tst_msg("Failed to ReadF from Memory IO");
		return TST_FAILED;
	}

	if (byte != 2) {
		tst_msg("Read wrong value byte = %u (expected 2)", byte);
		return TST_FAILED;
	}

	if (val != 0x0b0a) {
		tst_msg("Read wrong value = %04x (expected 0x0b0a)", val);
		return TST_FAILED;
	}

	return TST_SUCCESS;
}

static int test_IOMem(void)
{
	uint8_t buffer[128];
	unsigned int i;
	GP_IO *io;
	int ret;

	for (i = 0; i < sizeof(buffer); i++)
		buffer[i] = i;

	io = GP_IOMem(buffer, sizeof(buffer), NULL);

	if (!io) {
		tst_msg("Failed to initialize memory IO");
		return TST_FAILED;
	}

	ret = do_test(io);
	if (ret)
		return ret;

	if (GP_IOClose(io)) {
		tst_msg("Failed to close memory IO");
		return TST_FAILED;
	}

	return TST_SUCCESS;
}

#define TFILE "test.io"

static int test_IOFile(void)
{
	uint8_t buffer[128];
	unsigned int i;
	int ret;
	GP_IO *io;

	for (i = 0; i < sizeof(buffer); i++)
		buffer[i] = i;

	io = GP_IOFile(TFILE, GP_IO_WRONLY);

	if (!io) {
		tst_msg("Failed to open file IO for writing: %s",
		        strerror(errno));
		return TST_FAILED;
	}

	ret = GP_IOWrite(io, buffer, sizeof(buffer));

	if (ret != sizeof(buffer)) {
		tst_msg("Failed to write: %s", strerror(errno));
		return TST_FAILED;
	}

	if (GP_IOClose(io)) {
		tst_msg("Failed to close file IO: %s", strerror(errno));
		return TST_FAILED;
	}

	io = GP_IOFile(TFILE, GP_IO_RDONLY);

	if (!io) {
		tst_msg("Failed to open file IO for reading: %s",
		        strerror(errno));
		return TST_FAILED;
	}

	ret = do_test(io);
	if (ret)
		return ret;

	if (GP_IOClose(io)) {
		tst_msg("Failed to close file IO: %s", strerror(errno));
		return TST_FAILED;
	}

	return TST_SUCCESS;
}

static ssize_t test_IOFill_read(GP_IO GP_UNUSED(*io), void *buf, size_t size)
{
	ssize_t ret = GP_MIN(7u, size);

	memset(buf, 'a', ret);

	return ret;
}

static int try_IOFill_and_check(GP_IO *io, unsigned int size)
{
	uint8_t buf[125];
	unsigned int i, fail = 0;

	memset(buf, 0, sizeof(buf));

	if (GP_IOFill(io, buf, size)) {
		tst_msg("Failed to fill buffer size=%u: %s",
		        size, strerror(errno));
		return TST_FAILED;
	}

	for (i = 0; i < size; i++) {
		if (buf[i] != 'a') {
			tst_msg("Wrong data in buffer at %u", i);
			fail++;
		}
	}

	for (i = size; i < sizeof(buf); i++) {
		if (buf[i] != 0) {
			tst_msg("Wrong data in buffer at %u", i);
			fail++;
		}
	}

	if (fail)
		return TST_FAILED;

	return TST_SUCCESS;
}

static int test_IOFill(void)
{
	GP_IO io = {.Read = test_IOFill_read};
	int ret = 0;

	ret += try_IOFill_and_check(&io, 7);
	ret += try_IOFill_and_check(&io, 10);
	ret += try_IOFill_and_check(&io, 43);
	ret += try_IOFill_and_check(&io, 69);

	if (ret)
		return TST_FAILED;

	return TST_SUCCESS;
}

static size_t counter;

static ssize_t wbuf_write(GP_IO GP_UNUSED(*io), void *buf, size_t size)
{
	unsigned int i;

	for (i = 0; i < size; i++) {
		if (((uint8_t*)buf)[i] != 'a') {
			tst_msg("Wrong data in buffer");
			return -1;
		}
	}

	counter += size;
	return size;
}

static int wbuf_close(GP_IO GP_UNUSED(*io))
{
	return 0;
}

static int test_IOWBuffer(void)
{
	GP_IO *bio;
	size_t cnt = 0;
	unsigned int i;
	GP_IO io = {
		.Write = wbuf_write,
		.Close = wbuf_close,
	};

	counter = 0;

	bio = GP_IOWBuffer(&io, 100);

	if (!bio)
		return TST_FAILED;

	for (i = 0; i < 100; i++) {
		size_t to_write = i % 10 + 1;
		if (GP_IOFlush(bio, "aaaaaaaaaaa", to_write)) {
			tst_msg("Failed to write data: %s", tst_strerr(errno));
			GP_IOClose(bio);
			return TST_FAILED;
		}
		cnt += to_write;
	}

	if (GP_IOClose(bio)) {
		tst_msg("Failed to close I/O: %s", tst_strerr(errno));
		return TST_FAILED;
	}

	if (counter != cnt) {
		tst_msg("Wrong number of bytes written have %zu expected %zu",
		        counter, cnt);
		return TST_FAILED;
	}

	return TST_SUCCESS;
}


const struct tst_suite tst_suite = {
	.suite_name = "IO",
	.tests = {
		{.name = "IOMem",
		 .tst_fn = test_IOMem,
		 .flags = TST_CHECK_MALLOC},

		{.name = "IOFile",
		 .tst_fn = test_IOFile,
		 .flags = TST_CHECK_MALLOC | TST_TMPDIR},

		{.name = "IOFill",
		 .tst_fn = test_IOFill},

		{.name = "IOWBuffer",
		 .tst_fn = test_IOWBuffer},

		{.name = NULL},
	}
};
