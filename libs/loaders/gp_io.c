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
#include "core/gp_common.h"

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
	int err, flags;
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
		flags = O_CREAT | O_WRONLY;
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

int gp_io_mark(gp_io *self, enum gp_io_mark_types type)
{
	off_t ret;

	switch (type) {
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

#define TYPE(x) ((x) & GP_IO_TYPE_MASK)
#define VAL(x) ((x) & ~GP_IO_TYPE_MASK)

static void readf_size(uint16_t *types,
                       unsigned int *min_size, unsigned int *max_size)
{
	unsigned int min = 0;
	unsigned int max = 0;

	while (*types != GP_IO_END) {
		switch (TYPE(*types)) {
		case GP_IO_CONST:
		case GP_IO_BYTE:
			min++;
			max++;
		break;
		case GP_IO_L2:
		case GP_IO_B2:
			min += 2;
			max += 2;
		break;
		case GP_IO_L4:
		case GP_IO_B4:
			min += 4;
			max += 4;
		break;
		case GP_IO_ARRAY:
		case GP_IO_IGN:
			min += VAL(*types);
			max += VAL(*types);
		break;
		case GP_IO_PPSTR:
			min += 2;
			max += 255;
		break;
		}
		types++;
	}

	*min_size = min;
	*max_size = max;
}

static int needs_swap(uint16_t type)
{
#if __BYTE_ORDER == __BIG_ENDIAN
	if (type == GP_IO_L2 || type == GP_IO_L4)
		return 1;
	return 0;
#elif __BYTE_ORDER == __LITTLE_ENDIAN
	if (type == GP_IO_B2 || type == GP_IO_B4)
		return 1;
	return 0;
#else
# error Unknown Endianity
#endif
}

static void write_str(uint16_t type, uint8_t *dest,
                      uint8_t *src, unsigned int size)
{
	unsigned int dest_size = VAL(type);
	unsigned int i;

	if (!dest_size)
		return;

	for (i = 0; i < dest_size - 1 && i < size; i++)
		dest[i] = src[i];

	dest[i] = '\0';
}

int gp_io_readf(gp_io *self, uint16_t *types, ...)
{
	unsigned int read_size, buf_size, size;
	int ret;
	va_list va;
	uint8_t *ptr;

	readf_size(types, &read_size, &buf_size);

	if (!read_size)
		return 0;

	uint8_t buffer[buf_size], *buf = buffer;

	if (gp_io_fill(self, buf, read_size))
		return -1;

	ret = 0;
	va_start(va, types);

	while (*types != GP_IO_END) {
		switch (TYPE(*types)) {
		case GP_IO_CONST:
			//TODO: Seek back?
			if (VAL(*types) != *buf) {
				errno = EINVAL;
				goto end;
			}
			buf++;
		break;
		case GP_IO_BYTE:
			ptr = va_arg(va, uint8_t*);
			*ptr = *buf;
			buf++;
		break;
		case GP_IO_L2:
		case GP_IO_B2:
			ptr = va_arg(va, uint8_t*);

			if (needs_swap(*types)) {
				ptr[0] = buf[1];
				ptr[1] = buf[0];
			} else {
				ptr[0] = buf[0];
				ptr[1] = buf[1];
			}
			buf += 2;
		break;
		case GP_IO_L4:
		case GP_IO_B4:
			ptr = va_arg(va, uint8_t*);

			if (needs_swap(*types)) {
				ptr[0] = buf[3];
				ptr[1] = buf[2];
				ptr[2] = buf[1];
				ptr[3] = buf[0];
			} else {
				ptr[0] = buf[0];
				ptr[1] = buf[1];
				ptr[2] = buf[2];
				ptr[3] = buf[3];
			}
			buf += 4;
		break;
		case GP_IO_ARRAY:
			ptr = va_arg(va, void*);
			memcpy(ptr, buf, VAL(*types));
		/* fallthrough */
		case GP_IO_IGN:
			buf += VAL(*types);
		break;
		case GP_IO_PPSTR:
			ptr = va_arg(va, void*);
			size = *buf;

			/* empty string */
			if (!size) {
				write_str(*types, ptr, NULL, 0);
				buf += 2;
			} else {
				/* fill up another part of the buffer */
				if (gp_io_fill(self, buf + read_size, size))
					return -1;
				read_size += size;
				write_str(*types, ptr, buf + 1, size);
				buf += GP_ALIGN2(size + 1);
                        }
		break;
		}

		types++;
		ret++;
	}

end:
	va_end(va);
	return ret;
}

int gp_io_writef(gp_io *io, uint16_t *types, ...)
{
	va_list va;
	uint8_t *ptr, t;
	int32_t i4;
	int16_t i2;

	va_start(va, types);

	while (*types != GP_IO_END) {
		switch (TYPE(*types)) {
		case GP_IO_CONST:
			t = VAL(*types);
			if (gp_io_write(io, &t, 1) != 1)
				goto err;
		break;
		case GP_IO_L2:
		case GP_IO_B2:
			i2 = va_arg(va, int);
			ptr = (void*)&i2;

			if (needs_swap(*types))
				GP_SWAP(ptr[0], ptr[1]);

			if (gp_io_write(io, ptr, 2) != 2)
				goto err;
		break;
		case GP_IO_L4:
		case GP_IO_B4:
			i4 = va_arg(va, int);
			ptr = (void*)&i4;

			if (needs_swap(*types)) {
				GP_SWAP(ptr[0], ptr[3]);
				GP_SWAP(ptr[1], ptr[2]);
			}

			if (gp_io_write(io, ptr, 4) != 4)
				goto err;
		break;
		default:
			GP_WARN("Invalid type %"PRIu16"\n", *types);
			goto err;
		}
		types++;
	}

	va_end(va);
	return 0;
err:
	va_end(va);
	return -1;
}

int gp_io_read_b4(gp_io *io, uint32_t *val)
{
	uint16_t desc[] = {
		GP_IO_B4,
		GP_IO_END
	};

	return gp_io_readf(io, desc, val) != 1;
}

int gp_io_read_b2(gp_io *io, uint16_t *val)
{
	uint16_t desc[] = {
		GP_IO_B2,
		GP_IO_END
	};

	return gp_io_readf(io, desc, val) != 1;
}
