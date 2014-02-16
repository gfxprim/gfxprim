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

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>

#include <core/GP_ByteOrder.h>
#include <core/GP_Debug.h>
#include <core/GP_Common.h>

#include "loaders/GP_IO.h"

struct file_io {
	int fd;
};

static ssize_t file_read(GP_IO *self, void *buf, size_t size)
{
	struct file_io *file_io = GP_IO_PRIV(self);

	return read(file_io->fd, buf, size);
}

static ssize_t file_write(GP_IO *self, void *buf, size_t size)
{
	struct file_io *file_io = GP_IO_PRIV(self);

	return write(file_io->fd, buf, size);
}

static off_t file_seek(GP_IO *self, off_t off, enum GP_IOWhence whence)
{
	struct file_io *file_io = GP_IO_PRIV(self);

	switch (whence) {
	case GP_IO_SEEK_SET:
	case GP_IO_SEEK_CUR:
	case GP_IO_SEEK_END:
	break;
	default:
		GP_WARN("Invalid whence");
		return (off_t)-1;
	}

	return lseek(file_io->fd, off, whence);
}

static int file_close(GP_IO *self)
{
	struct file_io *file_io = GP_IO_PRIV(self);
	int fd;

	GP_DEBUG(1, "Closing IOFile");

	fd = file_io->fd;
	free(self);

	return close(fd);
}

GP_IO *GP_IOFile(const char *path, enum GP_IOFileMode mode)
{
	int err, flags;
	GP_IO *io;
	struct file_io *file_io;

	GP_DEBUG(1, "Creating IOFile '%s'", path);

	io = malloc(sizeof(GP_IO) + sizeof(struct file_io));

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

	io->Seek = file_seek;
	io->Read = file_read;
	io->Write = file_write;

	if (mode == GP_IO_RDONLY)
		io->Write = NULL;

	if (mode == GP_IO_WRONLY)
		io->Read = NULL;

	io->Close = file_close;

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

static ssize_t mem_read(GP_IO *io, void *buf, size_t size)
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

static off_t mem_seek(GP_IO *io, off_t off, enum GP_IOWhence whence)
{
	struct mem_io *mem_io = GP_IO_PRIV(io);

	switch (whence) {
	case GP_IO_SEEK_CUR:
		if (off + mem_io->pos > mem_io->size) {
			errno = EINVAL;
			return (off_t)-1;
		}

		mem_io->pos += off;
	break;
	case GP_IO_SEEK_SET:
		if (off < 0 || off > (off_t)mem_io->pos) {
			errno = EINVAL;
			return (off_t)-1;
		}
		mem_io->pos = off;
	break;
	case GP_IO_SEEK_END:
		if (off) {
			errno = EINVAL;
			return (off_t)-1;
		}
		mem_io->pos = mem_io->size;
	break;
	default:
		GP_WARN("Invalid whence");
		errno = EINVAL;
		return (off_t)-1;
	}

	return mem_io->pos;
}

static int mem_close(GP_IO *io)
{
	struct mem_io *mem_io = GP_IO_PRIV(io);

	GP_DEBUG(1, "Closing IOMem");

	if (mem_io->free)
		mem_io->free(mem_io->buf);

	free(io);

	return 0;
}

GP_IO *GP_IOMem(void *buf, size_t size, void (*free)(void *))
{
	GP_IO *io;
	struct mem_io *mem_io;

	GP_DEBUG(1, "Creating IOMem %p size=%zu", buf, size);

	io = malloc(sizeof(GP_IO) + sizeof(*mem_io));

	if (!io) {
		GP_DEBUG(1, "Malloc failed :(");
		errno = ENOMEM;
		return NULL;
	}

	io->Read = mem_read;
	io->Seek = mem_seek;
	io->Close = mem_close;
	io->Write = NULL;

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

	GP_IO *io;
};

static ssize_t sub_read(GP_IO *io, void *buf, size_t size)
{
	struct sub_io *sub_io = GP_IO_PRIV(io);

	if (sub_io->cur > sub_io->end) {
		GP_BUG("Current offset (%zi) is after the end (%zi)",
		       sub_io->cur, sub_io->end);
		errno = EINVAL;
		return 0;
	}

	size_t io_size = sub_io->end - sub_io->cur;

	size = GP_MIN(size, io_size);

	if (size == 0)
		return 0;

	ssize_t ret = GP_IORead(sub_io->io, buf, size);

	if (ret < 0)
		return ret;

	sub_io->cur += ret;
	return ret;
}

static off_t sub_seek(GP_IO *io, off_t off, enum GP_IOWhence whence)
{
	struct sub_io *sub_io = GP_IO_PRIV(io);
	off_t io_size, ret, poff;

	switch (whence) {
	case GP_IO_SEEK_CUR:
		//TODO: Overflow
		poff = sub_io->cur + off;

		if (poff < sub_io->start || poff > sub_io->end) {
			errno = EINVAL;
			return (off_t)-1;
		}

		ret = GP_IOSeek(sub_io->io, off, whence);
	break;
	case GP_IO_SEEK_SET:
		io_size = sub_io->end - sub_io->start;

		if (off > io_size || off < 0) {
			errno = EINVAL;
			return (off_t)-1;
		}

		ret = GP_IOSeek(sub_io->io, sub_io->start + off, whence);
	break;
	case GP_IO_SEEK_END:
		io_size = sub_io->end - sub_io->start;

		if (off + io_size < 0 || off > 0) {
			errno = EINVAL;
			return (off_t)-1;
		}

		ret = GP_IOSeek(sub_io->io, sub_io->end + off, whence);
	break;
	default:
		GP_WARN("Invalid whence");
		errno = EINVAL;
		return (off_t)-1;
	}

	if (ret == (off_t)-1)
		return (off_t)-1;

	sub_io->cur = ret;

	return sub_io->cur - sub_io->start;
}

static int sub_close(GP_IO *io)
{
	struct sub_io *sub_io = GP_IO_PRIV(io);

	GP_DEBUG(1, "Closing SubIO (from %p)", sub_io->io);

	free(io);

	return 0;
}

GP_IO *GP_IOSubIO(GP_IO *pio, size_t size)
{
	GP_IO *io;
	struct sub_io *sub_io;

	GP_DEBUG(1, "Creating SubIO (from %p) size=%zu", pio, size);

	io = malloc(sizeof(GP_IO) + sizeof(*sub_io));

	if (!io) {
		GP_DEBUG(1, "Malloc failed :(");
		errno = ENOMEM;
		return NULL;
	}

	io->Read = sub_read;
	io->Seek = sub_seek;
	io->Close = sub_close;
	io->Write = NULL;

	sub_io = GP_IO_PRIV(io);
	sub_io->cur = sub_io->start = GP_IOTell(pio);

	//TODO: Overflow
	sub_io->end = sub_io->start + size;
	sub_io->io = pio;

	return io;
}

int GP_IOMark(GP_IO *self, enum GP_IOMarkTypes type)
{
	off_t ret;

	switch (type) {
	case GP_IO_MARK:
		ret = GP_IOSeek(self, 0, GP_IO_SEEK_CUR);
	break;
	case GP_IO_REWIND:
		ret = GP_IOSeek(self, self->mark, SEEK_SET);
	break;
	default:
		GP_WARN("Invalid mark type");
		return -1;
	}

	if (ret == (off_t)-1) {
		GP_WARN("Failed to lseek IO Stream");
		return -1;
	}

	self->mark = ret;
	return 0;
}

off_t GP_IOSize(GP_IO *io)
{
	off_t cur = GP_IOTell(io);
	off_t ret;

	ret = GP_IOSeek(io, 0, GP_IO_SEEK_END);

	if (ret == (off_t)-1)
		return ret;

	GP_IOSeek(io, cur, GP_IO_SEEK_SET);

	GP_DEBUG(2, "IO Size = %lli", (long long)ret);

	return ret;
}

int GP_IOFill(GP_IO *io, void *buf, size_t size)
{
	size_t read = 0;
	int ret;

	do {
		ret = GP_IORead(io, (char*)buf + read, size - read);

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

int GP_IOReadF(GP_IO *self, uint16_t *types, ...)
{
	unsigned int read_size, buf_size, size;
	int ret;
	va_list va;
	uint8_t *ptr;

	readf_size(types, &read_size, &buf_size);

	if (!read_size)
		return 0;

	uint8_t buffer[buf_size], *buf = buffer;

	if (GP_IOFill(self, buf, read_size))
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
				if (GP_IOFill(self, buf + read_size, size))
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

int GP_IOReadB4(GP_IO *io, uint32_t *val)
{
	uint16_t desc[] = {
		GP_IO_B4,
		GP_IO_END
	};

	return GP_IOReadF(io, desc, val) != 1;
}

int GP_IOReadB2(GP_IO *io, uint16_t *val)
{
	uint16_t desc[] = {
		GP_IO_B2,
		GP_IO_END
	};

	return GP_IOReadF(io, desc, val) != 1;
}
