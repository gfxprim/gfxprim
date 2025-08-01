// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include <inttypes.h>

#include <core/gp_byte_order.h>
#include <core/gp_debug.h>
#include <core/gp_common.h>

#include <loaders/gp_io.h>

struct file_io {
	int fd;
};

static ssize_t file_read(gp_io *self, void *buf, size_t size)
{
	struct file_io *file_io = GP_IO_PRIV(self);

	return read(file_io->fd, buf, size);
}

static ssize_t file_write(gp_io *self, const void *buf, size_t size)
{
	struct file_io *file_io = GP_IO_PRIV(self);

	return write(file_io->fd, buf, size);
}

static off_t file_seek(gp_io *self, off_t off, enum gp_seek_whence whence)
{
	struct file_io *file_io = GP_IO_PRIV(self);

	switch (whence) {
	case GP_SEEK_SET:
	case GP_SEEK_CUR:
	case GP_SEEK_END:
	break;
	default:
		GP_WARN("Invalid whence");
		errno = EINVAL;
		return -1;
	}

	return lseek(file_io->fd, off, whence);
}

static int file_close(gp_io *self)
{
	struct file_io *file_io = GP_IO_PRIV(self);
	int fd;

	GP_DEBUG(1, "Closing IOFile");

	fd = file_io->fd;
	free(self);

	return close(fd);
}

gp_io *gp_io_file(const char *path, enum gp_io_file_mode mode)
{
	int err, flags = 0;
	gp_io *io;
	struct file_io *file_io;

	GP_DEBUG(1, "Creating IOFile '%s'", path);

	io = malloc(sizeof(gp_io) + sizeof(struct file_io));

	if (!io) {
		GP_DEBUG(1, "Malloc failed :(");
		err = ENOMEM;
		goto err0;
	}

	switch (mode) {
	case GP_IO_WRONLY:
		flags = O_CREAT | O_WRONLY | O_TRUNC;
	break;
	case GP_IO_RDONLY:
		flags = O_RDONLY;
	break;
	case GP_IO_RDWR:
		flags = O_CREAT | O_RDWR;
	break;
	}

	file_io = GP_IO_PRIV(io);
	file_io->fd = open(path, flags, 0666);

	if (file_io->fd < 0) {
		err = errno;
		GP_DEBUG(1, "Failed to open '%s': %s", path, strerror(errno));
		goto err1;
	}

	io->mark = 0;

	io->seek = file_seek;
	io->read = file_read;
	io->write = file_write;

	if (mode == GP_IO_RDONLY)
		io->write = NULL;

	if (mode == GP_IO_WRONLY)
		io->read = NULL;

	io->close = file_close;

	return io;
err1:
	free(io);
err0:
	errno = err;
	return NULL;
}

struct mem_io {
	uint8_t *buf;
	size_t size;
	size_t pos;
	void (*free)(void *);
};

static ssize_t mem_read(gp_io *io, void *buf, size_t size)
{
	struct mem_io *mem_io = GP_IO_PRIV(io);
	size_t rest = mem_io->size - mem_io->pos;
	ssize_t ret = GP_MIN(rest, size);

	if (ret <= 0) {
		errno = EIO;
		return 0;
	}

	memcpy(buf, mem_io->buf + mem_io->pos, ret);
	mem_io->pos += ret;

	return ret;
}

static off_t mem_seek(gp_io *io, off_t off, enum gp_seek_whence whence)
{
	struct mem_io *mem_io = GP_IO_PRIV(io);

	switch (whence) {
	case GP_SEEK_CUR:
		if (-off > (off_t)mem_io->pos ||
		     off + mem_io->pos > mem_io->size) {
			errno = EINVAL;
			return -1;
		}

		mem_io->pos += off;
	break;
	case GP_SEEK_SET:
		if (off < 0 || off > (off_t)mem_io->size) {
			errno = EINVAL;
			return -1;
		}
		mem_io->pos = off;
	break;
	case GP_SEEK_END:
		if (off > 0 || off + (off_t)mem_io->size < 0) {
			errno = EINVAL;
			return -1;
		}
		mem_io->pos = mem_io->size + off;
	break;
	default:
		GP_WARN("Invalid whence");
		errno = EINVAL;
		return -1;
	}

	return mem_io->pos;
}

static int mem_close(gp_io *io)
{
	struct mem_io *mem_io = GP_IO_PRIV(io);

	GP_DEBUG(1, "Closing IOMem");

	if (mem_io->free)
		mem_io->free(mem_io->buf);

	free(io);

	return 0;
}

gp_io *gp_io_mem(void *buf, size_t size, void (*free)(void *))
{
	gp_io *io;
	struct mem_io *mem_io;

	GP_DEBUG(1, "Creating IOMem %p size=%zu", buf, size);

	io = malloc(sizeof(gp_io) + sizeof(*mem_io));

	if (!io) {
		GP_DEBUG(1, "Malloc failed :(");
		errno = ENOMEM;
		return NULL;
	}

	io->read = mem_read;
	io->seek = mem_seek;
	io->close = mem_close;
	io->write = NULL;

	mem_io = GP_IO_PRIV(io);

	mem_io->free = free;
	mem_io->buf = buf;
	mem_io->size = size;
	mem_io->pos = 0;

	return io;
}

struct sub_io {
	/* Points to parent IO */
	off_t start;
	off_t end;
	off_t cur;

	gp_io *io;
};

static ssize_t sub_read(gp_io *io, void *buf, size_t size)
{
	struct sub_io *sub_io = GP_IO_PRIV(io);

	if (sub_io->cur > sub_io->end) {
		GP_BUG("Current offset (%zi) is after the end (%zi)",
		       (ssize_t)sub_io->cur, (ssize_t)sub_io->end);
		errno = EINVAL;
		return 0;
	}

	size_t io_size = sub_io->end - sub_io->cur;

	size = GP_MIN(size, io_size);

	if (size == 0)
		return 0;

	ssize_t ret = gp_io_read(sub_io->io, buf, size);

	if (ret < 0)
		return ret;

	sub_io->cur += ret;
	return ret;
}

static off_t sub_seek(gp_io *io, off_t off, enum gp_seek_whence whence)
{
	struct sub_io *sub_io = GP_IO_PRIV(io);
	off_t io_size, ret, poff;

	switch (whence) {
	case GP_SEEK_CUR:
		//TODO: Overflow
		poff = sub_io->cur + off;

		if (poff < sub_io->start || poff > sub_io->end) {
			errno = EINVAL;
			return -1;
		}

		ret = gp_io_seek(sub_io->io, off, whence);
	break;
	case GP_SEEK_SET:
		io_size = sub_io->end - sub_io->start;

		if (off > io_size || off < 0) {
			errno = EINVAL;
			return -1;
		}

		ret = gp_io_seek(sub_io->io, sub_io->start + off, whence);
	break;
	case GP_SEEK_END:
		io_size = sub_io->end - sub_io->start;

		if (off + io_size < 0 || off > 0) {
			errno = EINVAL;
			return -1;
		}

		ret = gp_io_seek(sub_io->io, sub_io->end + off, GP_SEEK_SET);
	break;
	default:
		GP_WARN("Invalid whence");
		errno = EINVAL;
		return -1;
	}

	if (ret == -1)
		return -1;

	sub_io->cur = ret;

	return sub_io->cur - sub_io->start;
}

static int sub_close(gp_io *io)
{
	struct sub_io *sub_io = GP_IO_PRIV(io);

	GP_DEBUG(1, "Closing SubIO (from %p)", sub_io->io);

	free(io);

	return 0;
}

gp_io *gp_io_sub_io(gp_io *pio, size_t size)
{
	gp_io *io;
	struct sub_io *sub_io;

	GP_DEBUG(1, "Creating SubIO (from %p) size=%zu", pio, size);

	io = malloc(sizeof(gp_io) + sizeof(*sub_io));

	if (!io) {
		GP_DEBUG(1, "Malloc failed :(");
		errno = ENOMEM;
		return NULL;
	}

	io->read = sub_read;
	io->seek = sub_seek;
	io->close = sub_close;
	io->write = NULL;

	sub_io = GP_IO_PRIV(io);
	sub_io->cur = sub_io->start = gp_io_tell(pio);

	//TODO: Overflow
	sub_io->end = sub_io->start + size;
	sub_io->io = pio;

	return io;
}

struct buf_io {
	gp_io *io;
	size_t bsize;
	size_t bpos;
	uint8_t buf[];
};

static int wbuf_close(gp_io *io)
{
	struct buf_io *buf_io = GP_IO_PRIV(io);
	int ret = 0;

	GP_DEBUG(1, "Closing BufferIO (from %p)", buf_io->io);

	if (buf_io->bpos) {
		if (gp_io_flush(buf_io->io, buf_io->buf, buf_io->bpos))
			ret = 1;
	}

	free(io);
	return ret;
}

static ssize_t buf_write(gp_io *io, const void *buf, size_t size)
{
	struct buf_io *buf_io = GP_IO_PRIV(io);
	size_t bfree = buf_io->bsize - buf_io->bpos;

	if (bfree < size) {
		GP_DEBUG(1, "Flusing BufferIO (%p)", io);
		if (gp_io_flush(buf_io->io, buf_io->buf, buf_io->bpos))
			return -1;
		buf_io->bpos = 0;
	}

	if (size > buf_io->bsize) {
		GP_DEBUG(1, "Buffer too large, doing direct write (%p)", io);
		if (gp_io_flush(buf_io->io, buf, size))
			return -1;
		return size;
	}

	memcpy(buf_io->buf + buf_io->bpos, buf, size);
	buf_io->bpos += size;
	return size;
}

gp_io *gp_io_wbuffer(gp_io *pio, size_t bsize)
{
	gp_io *io;
	struct buf_io *buf_io;

	if (!bsize)
		bsize = 512;

	GP_DEBUG(1, "Creating IOWBuffer (from %p) size=%zu", pio, bsize);

	//TODO: Do not create buffer IO for MemIO, just copy the callbacks to new IO
	io = malloc(sizeof(gp_io) + sizeof(*buf_io) + bsize);

	if (!io)
		return NULL;

	io->write = buf_write;
	io->close = wbuf_close;
	io->read = NULL;
	io->seek = NULL;

	buf_io = GP_IO_PRIV(io);
	buf_io->io = pio;
	buf_io->bsize = bsize;
	buf_io->bpos = 0;

	return io;
}

int gp_io_mark(gp_io *self, enum gp_io_mark_op op)
{
	off_t ret;

	switch (op) {
	case GP_IO_MARK:
		ret = gp_io_seek(self, 0, GP_SEEK_CUR);
	break;
	case GP_IO_REWIND:
		ret = gp_io_seek(self, self->mark, SEEK_SET);
	break;
	default:
		GP_WARN("Invalid mark type");
		return -1;
	}

	if (ret == -1) {
		GP_WARN("Failed to seek I/O Stream");
		return -1;
	}

	self->mark = ret;
	return 0;
}

off_t gp_io_size(gp_io *io)
{
	off_t cur = gp_io_tell(io);
	off_t ret;

	ret = gp_io_seek(io, 0, GP_SEEK_END);

	if (ret == -1)
		return ret;

	gp_io_seek(io, cur, GP_SEEK_SET);

	GP_DEBUG(2, "I/O Size = %lli", (long long)ret);

	return ret;
}

int gp_io_fill(gp_io *io, void *buf, size_t size)
{
	size_t read = 0;
	int ret;

	do {
		ret = gp_io_read(io, (char*)buf + read, size - read);

		if (ret <= 0) {
			/* end of file */
			if (ret == 0)
				errno = EIO;

			GP_DEBUG(1, "Failed to fill buffer: %s",
			         strerror(errno));
			return 1;
		}

		read += ret;

	} while (read < size);

	return 0;
}

int gp_io_flush(gp_io *io, const void *buf, size_t size)
{
	size_t wrote = 0;
	int ret;

	do {
		ret = gp_io_write(io, (char*)buf + wrote, size - wrote);

		if (ret <= 0) {
			GP_DEBUG(1, "Failed to flush buffer: %s",
			         strerror(errno));
			return 1;
		}

		wrote += ret;

	} while (wrote < size);

	return 0;
}

int gp_io_printf(gp_io *io, const char *fmt, ...)
{
	va_list va, vac;
	size_t size;
	int ret;
	char buf[1024];
	char *bufp = buf;

	va_start(va, fmt);
	va_copy(vac, va);
	size = vsnprintf(buf, sizeof(buf), fmt, vac);
	va_end(vac);

	if (size >= sizeof(buf)) {
		bufp = malloc(size+1);
		if (!bufp)
			return 1;

		vsnprintf(bufp, size, fmt, va);
	}

	ret = gp_io_flush(io, bufp, size);

	if (size >= sizeof(buf))
		free(bufp);

	va_end(va);

	return ret;
}
