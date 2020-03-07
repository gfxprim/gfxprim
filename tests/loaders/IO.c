// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <errno.h>

#include "core/gp_common.h"
#include <loaders/gp_io.h>

#include "tst_test.h"

static const char *whence_name(int whence)
{
	switch (whence) {
	case GP_IO_SEEK_SET:
		return "SEEK_SET";
	case GP_IO_SEEK_CUR:
		return "SEEK_CUR";
	case GP_IO_SEEK_END:
		return "SEEK_END";
	}

	return "INVALID";
}

static int seek_and_tell(gp_io *io, off_t off, int whence, off_t exp_off)
{
	if (gp_io_seek(io, off, whence) == -1) {
		tst_msg("Failed to seek %zi %s", off, whence_name(whence));
		return 1;
	}

	off = gp_io_tell(io);

	if (off != exp_off) {
		tst_msg("Wrong offset %zi expected %zi", off, exp_off);
		return 1;
	}

	return 0;
}

static int seek_fail(gp_io *io, off_t off, int whence)
{
	off_t ret, start, end;

	start = gp_io_tell(io);

	ret = gp_io_seek(io, off, whence);

	if (ret != -1) {
		tst_msg("Seek succeded unexpectedly %zi %s",
		        off, whence_name(whence));
		return 1;
	}

	end = gp_io_tell(io);

	if (start != end) {
		tst_msg("Seek %zi %s failed but offset changed %zi -> %zi",
		        off, whence_name(whence), start, end);
		return 1;
	}

	return 0;
}

/*
 * Expects IO buffer filled with monotonically increasing bytes, i.e.
 * 0x00 0x01 0x02 ...
 */
static int do_test(gp_io *io, off_t io_size, int is_file)
{
	int ret;
	uint8_t buf[10];
	unsigned int i;
	off_t off;
	int failed = 0;

	off = gp_io_tell(io);

	if (off != 0) {
		tst_msg("Wrong offset before first read %zu", off);
		return TST_FAILED;
	}

	ret = gp_io_read(io, buf, 10);

	if (ret != 10) {
		tst_msg("First I/O read failed");
		return TST_FAILED;
	}

	for (i = 0; i < 10; i++) {
		if (i != buf[i]) {
			tst_msg("Read wrong data at %u", i);
			return TST_FAILED;
		}
	}

	off = gp_io_tell(io);

	if (off != 10) {
		tst_msg("Have wrong offset %zu, after read 10", off);
		return TST_FAILED;
	}


	ret = gp_io_read(io, buf, 10);

	if (ret != 10) {
		tst_msg("Second I/O read failed");
		return TST_FAILED;
	}

	for (i = 0; i < 10; i++) {
		if (i + 10 != buf[i]) {
			tst_msg("Read wrong data at %u", i + 10);
			return TST_FAILED;
		}
	}

	if (gp_io_rewind(io)) {
		tst_msg("Failed to rewind to start");
		return TST_FAILED;
	}

	ret = gp_io_tell(io);

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

	if (gp_io_readf(io, header, &byte, &val) != 5) {
		tst_msg("Failed to ReadF from I/O");
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

	/* Seek tests */
	if (seek_and_tell(io, 0, GP_IO_SEEK_END, io_size))
		failed++;

	if (seek_and_tell(io, -io_size, GP_IO_SEEK_END, 0))
		failed++;

	if (seek_and_tell(io, io_size, GP_IO_SEEK_SET, io_size))
		failed++;

	if (seek_and_tell(io, 0, GP_IO_SEEK_SET, 0))
		failed++;

	if (seek_and_tell(io, io_size, GP_IO_SEEK_CUR, io_size))
		failed++;

	if (seek_and_tell(io, -io_size, GP_IO_SEEK_CUR, 0))
		failed++;

	if (seek_fail(io, -1, GP_IO_SEEK_CUR))
		failed++;

	if (!is_file && seek_fail(io, io_size+1, GP_IO_SEEK_CUR))
		failed++;

	if (seek_fail(io, -1, GP_IO_SEEK_SET))
		failed++;

	if (!is_file && seek_fail(io, io_size + 1, GP_IO_SEEK_SET))
		failed++;

	if (!is_file && seek_fail(io, 1, GP_IO_SEEK_END))
		failed++;

	if (seek_fail(io, -io_size - 1, GP_IO_SEEK_END))
		failed++;

	if (seek_fail(io, 0, 100))
		failed++;

	if (failed)
		return TST_FAILED;

	return TST_SUCCESS;
}

static int test_IOMem(void)
{
	uint8_t buffer[128];
	unsigned int i;
	gp_io *io;
	int ret;

	for (i = 0; i < sizeof(buffer); i++)
		buffer[i] = i;

	io = gp_io_mem(buffer, sizeof(buffer), NULL);

	if (!io) {
		tst_msg("Failed to initialize memory I/O");
		return TST_FAILED;
	}

	ret = do_test(io, sizeof(buffer), 0);
	if (ret) {
		gp_io_close(io);
		return ret;
	}

	if (gp_io_close(io)) {
		tst_msg("Failed to close memory I/O");
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
	gp_io *io;

	for (i = 0; i < sizeof(buffer); i++)
		buffer[i] = i;

	io = gp_io_file(TFILE, GP_IO_WRONLY);

	if (!io) {
		tst_msg("Failed to open file I/O for writing: %s",
		        strerror(errno));
		return TST_FAILED;
	}

	ret = gp_io_write(io, buffer, sizeof(buffer));

	if (ret != sizeof(buffer)) {
		tst_msg("Failed to write: %s", strerror(errno));
		return TST_FAILED;
	}

	if (gp_io_close(io)) {
		tst_msg("Failed to close file I/O: %s", strerror(errno));
		return TST_FAILED;
	}

	io = gp_io_file(TFILE, GP_IO_RDONLY);

	if (!io) {
		tst_msg("Failed to open file I/O for reading: %s",
		        strerror(errno));
		return TST_FAILED;
	}

	ret = do_test(io, sizeof(buffer), 1);
	if (ret) {
		gp_io_close(io);
		return ret;
	}

	if (gp_io_close(io)) {
		tst_msg("Failed to close file I/O: %s", strerror(errno));
		return TST_FAILED;
	}

	return TST_SUCCESS;
}

static int test_IOSubIO(void)
{
	uint8_t buffer[128];
	unsigned int i;
	gp_io *io, *pio;
	off_t off;
	int ret;

	for (i = 0; i < sizeof(buffer); i++)
		buffer[i] = i;

	pio = gp_io_mem(buffer, sizeof(buffer), NULL);

	if (!pio) {
		tst_msg("Failed to initialize memory I/O");
		return TST_FAILED;
	}

	io = gp_io_sub_io(pio, 100);

	if (!io) {
		tst_msg("Failed to initialize sub I/O");
		gp_io_close(io);
		return TST_FAILED;
	}

	ret = do_test(io, 100, 0);
	if (ret) {
		gp_io_close(pio);
		gp_io_close(io);
		return ret;
	}

	if (gp_io_seek(io, 0, GP_IO_SEEK_END) == (off_t)-1) {
		tst_msg("Failed to seek to the end of sub I/O: %s",
		        tst_strerr(errno));
		goto failed;
	}

	ret = gp_io_read(io, buffer, sizeof(buffer));

	if (ret != 0) {
		tst_msg("Read at the end of sub I/O returned %i", ret);
		goto failed;
	}

	off = gp_io_tell(pio);

	if (off != 100) {
		tst_msg("Wrong offset at the parent I/O: %zu", off);
		goto failed;
	}

	if (gp_io_close(io)) {
		tst_msg("Failed to close sub I/O");
		gp_io_close(pio);
		return TST_FAILED;
	}

	if (gp_io_close(pio)) {
		tst_msg("Failed to close memory I/O");
		return TST_FAILED;
	}

	return TST_SUCCESS;
failed:
	gp_io_close(io);
	gp_io_close(pio);
	return TST_FAILED;
}

static ssize_t test_IOFill_read(gp_io GP_UNUSED(*io), void *buf, size_t size)
{
	ssize_t ret = GP_MIN(7u, size);

	memset(buf, 'a', ret);

	return ret;
}

static int try_IOFill_and_check(gp_io *io, unsigned int size)
{
	uint8_t buf[125];
	unsigned int i, fail = 0;

	memset(buf, 0, sizeof(buf));

	if (gp_io_fill(io, buf, size)) {
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
	gp_io io = {.read = test_IOFill_read};
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

static ssize_t flush_write(gp_io GP_UNUSED(*io), const void *buf, size_t size)
{
	size_t to_write = GP_MIN(7u, size);

	if (((uint8_t*)buf)[0] != counter)
		return -1;

	counter += to_write;

	return to_write;
}

static int test_IOFlush(void)
{
	gp_io io = {.write = flush_write};
	unsigned int i;
	int fail = 0;
	uint8_t buf[255];

	for (i = 0; i < 255; i++)
		buf[i] = i;

	for (i = 1; i < 255; i++) {
		counter = 0;
		if (gp_io_flush(&io, buf, i)) {
			if (!fail)
				tst_msg("gp_io_flush failed");
			fail++;
		}

		if (counter != i) {
			if (!fail)
				tst_msg("Bytes written %zu expected %u",
				        counter, i);
			fail++;
		}
	}

	if (fail)
		return TST_FAILED;

	return TST_SUCCESS;
}

static ssize_t wbuf_write(gp_io GP_UNUSED(*io), const void *buf, size_t size)
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

static int wbuf_close(gp_io GP_UNUSED(*io))
{
	return 0;
}

static int test_IOWBuffer(void)
{
	gp_io *bio;
	size_t cnt = 0;
	unsigned int i;
	gp_io io = {
		.write = wbuf_write,
		.close = wbuf_close,
	};

	counter = 0;

	bio = gp_io_wbuffer(&io, 100);

	if (!bio)
		return TST_FAILED;

	for (i = 0; i < 100; i++) {
		size_t to_write = i % 10 + 1;
		if (gp_io_flush(bio, "aaaaaaaaaaa", to_write)) {
			tst_msg("Failed to write data: %s", tst_strerr(errno));
			gp_io_close(bio);
			return TST_FAILED;
		}
		cnt += to_write;
	}

	if (gp_io_close(bio)) {
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

		{.name = "IOSubIO",
		 .tst_fn = test_IOSubIO,
		 .flags = TST_CHECK_MALLOC},

		{.name = "IOFile",
		 .tst_fn = test_IOFile,
		 .flags = TST_CHECK_MALLOC | TST_TMPDIR},

		{.name = "IOFill",
		 .tst_fn = test_IOFill},

		{.name = "IOFlush",
		 .tst_fn = test_IOFlush},

		{.name = "IOWBuffer",
		 .tst_fn = test_IOWBuffer,
		 .flags = TST_CHECK_MALLOC},

		{.name = NULL},
	}
};
