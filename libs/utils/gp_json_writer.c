// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2021-2022 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#include <errno.h>

#include <utils/gp_json_writer.h>
#include <utils/gp_vec_str.h>

static inline int get_depth_bit(gp_json_writer *self, char *mask)
{
	int depth = self->depth - 1;

	if (depth < 0)
		return -1;

	return !!(mask[depth/8] & (1<<(depth%8)));
}

static inline void set_depth_bit(gp_json_writer *self, int val)
{
	if (val)
		self->depth_type[self->depth/8] |= (1<<(self->depth%8));
	else
		self->depth_type[self->depth/8] &= ~(1<<(self->depth%8));

	self->depth_first[self->depth/8] |= (1<<(self->depth%8));

	self->depth++;
}

static inline void clear_depth_bit(gp_json_writer *self)
{
	self->depth--;
}

static inline int in_arr(gp_json_writer *self)
{
	return !get_depth_bit(self, self->depth_type);
}

static inline int in_obj(gp_json_writer *self)
{
	return get_depth_bit(self, self->depth_type);
}

static inline void clear_depth_first(gp_json_writer *self)
{
	int depth = self->depth - 1;

	self->depth_first[depth/8] &= ~(1<<(depth%8));
}

static inline int is_first(gp_json_writer *self)
{
	int ret = get_depth_bit(self, self->depth_first);

	if (ret == 1)
		clear_depth_first(self);

	return ret;
}

static inline void err(gp_json_writer *buf, const char *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	vsnprintf(buf->err, GP_JSON_ERR_MAX, fmt, va);
	va_end(va);
}

static inline int is_err(gp_json_writer *buf)
{
	return buf->err[0];
}

static inline int out(gp_json_writer *self, const char *buf, size_t len)
{
	return self->out(self, buf, len);
}

static inline int out_str(gp_json_writer *self, const char *str)
{
	return out(self, str, strlen(str));
}

static inline int out_ch(gp_json_writer *self, char ch)
{
	return out(self, &ch, 1);
}

#define ESC_FLUSH(esc_char) do {\
	out(self, val, i); \
	val += i + 1; \
	i = 0; \
	out_str(self, esc_char); \
} while (0)

static inline int out_esc_str(gp_json_writer *self, const char *val)
{
	if (out_ch(self, '"'))
		return 1;

	size_t i = 0;
	int8_t next_chsz;

	do {
		next_chsz = gp_utf8_next_chsz(val, i);

		if (next_chsz == 1) {
			switch (val[i]) {
			case '\"':
				ESC_FLUSH("\\\"");
			break;
			case '\\':
				ESC_FLUSH("\\\\");
			break;
			case '/':
				ESC_FLUSH("\\/");
			break;
			case '\b':
				ESC_FLUSH("\\b");
			break;
			case '\f':
				ESC_FLUSH("\\f");
			break;
			case '\n':
				ESC_FLUSH("\\n");
			break;
			case '\r':
				ESC_FLUSH("\\r");
			break;
			case '\t':
				ESC_FLUSH("\\t");
			break;
			default:
				i += next_chsz;
			}
		} else {
			i += next_chsz;
		}
	} while (next_chsz);

	if (i) {
		if (out(self, val, i))
			return 1;
	}

	if (out_ch(self, '"'))
		return 1;

	return 0;
}

static int do_padd(gp_json_writer *self)
{
	unsigned int i;

	for (i = 0; i < self->depth; i++) {
		if (out_ch(self, ' '))
			return 1;
	}

	return 0;
}

static int newline(gp_json_writer *self)
{
	if (out_ch(self, '\n'))
		return 0;

	if (do_padd(self))
		return 1;

	return 0;
}

static int add_common(gp_json_writer *self, const char *id)
{
	if (is_err(self))
		return 1;

	if (!self->depth) {
		err(self, "Object/Array has to be started first");
		return 1;
	}

	if (in_arr(self)) {
		if (id) {
			err(self, "Array entries can't have id");
			return 1;
		}
	} else {
		if (!id) {
			err(self, "Object entries must have id");
			return 1;
		}
	}

	if (!is_first(self) && out_ch(self, ','))
		return 1;

	if (self->depth && newline(self))
		return 1;

	if (id) {
		if (out_esc_str(self, id))
			return 1;

		if (out_str(self, ": "))
			return 1;
	}

	return 0;
}

int gp_json_obj_start(gp_json_writer *self, const char *id)
{
	if (self->depth >= GP_JSON_RECURSION_MAX)
		return 1;

	if (!self->depth && id) {
		err(self, "Top level object cannot have id");
		return 1;
	}

	if (self->depth && add_common(self, id))
		return 1;

	if (out_ch(self, '{'))
		return 1;

	set_depth_bit(self, 1);

	return 0;
}

int gp_json_obj_finish(gp_json_writer *self)
{
	if (is_err(self))
		return 1;

	if (!in_obj(self)) {
		err(self, "Not in object!");
		return 1;
	}

	int first = is_first(self);

	clear_depth_bit(self);

	if (!first)
		newline(self);

	return out_ch(self, '}');
}

int gp_json_arr_start(gp_json_writer *self, const char *id)
{
	if (self->depth >= GP_JSON_RECURSION_MAX) {
		err(self, "Recursion too deep");
		return 1;
	}

	if (!self->depth && id) {
		err(self, "Top level array cannot have id");
		return 1;
	}

	if (self->depth && add_common(self, id))
		return 1;

	if (out_ch(self, '['))
		return 1;

	set_depth_bit(self, 0);

	return 0;
}

int gp_json_arr_finish(gp_json_writer *self)
{
	if (is_err(self))
		return 1;

	if (!in_arr(self)) {
		err(self, "Not in array!");
		return 1;
	}

	int first = is_first(self);

	clear_depth_bit(self);

	if (!first)
		newline(self);

	return out_ch(self, ']');
}

int gp_json_null_add(gp_json_writer *self, const char *id)
{
	if (add_common(self, id))
		return 1;

	return out_str(self, "null");
}

int gp_json_int_add(gp_json_writer *self, const char *id, long val)
{
	char buf[64];

	if (add_common(self, id))
		return 1;

	snprintf(buf, sizeof(buf), "%li", val);

	return out_str(self, buf);
}

int gp_json_bool_add(gp_json_writer *self, const char *id, int val)
{
	if (add_common(self, id))
		return 1;

	if (val)
		return out_str(self, "true");
	else
		return out_str(self, "false");
}

int gp_json_str_add(gp_json_writer *self, const char *id, const char *val)
{
	if (add_common(self, id))
		return 1;

	if (out_esc_str(self, val))
		return 1;

	return 0;
}

int gp_json_float_add(gp_json_writer *self, const char *id, double val)
{
	char buf[64];

	if (add_common(self, id))
		return 1;

	snprintf(buf, sizeof(buf), "%lg", val);

	return out_str(self, buf);
}

int gp_json_finish(gp_json_writer *self)
{
	if (is_err(self))
		goto err;

	if (self->depth) {
		err(self, "Objects and/or Arrays not finished");
		goto err;
	}

	if (newline(self))
		return 1;

	return 0;
err:
	if (self->err_print)
		self->err_print(self->err_print_priv, self->err);

	return 1;
}

static int out_vec(gp_json_writer *self, const char *buf, size_t buf_len)
{
	char *vec = self->out_priv;
	size_t pos = gp_vec_len(vec) - 1;

	vec = gp_vec_expand(vec, buf_len);
	if (!vec) {
		err(self, "Allocation failure!");
		return 1;
	}

	memcpy(vec + pos, buf, buf_len);
	vec[gp_vec_len(vec)-1] = 0;
	self->out_priv = vec;

	return 0;
}

gp_json_writer *gp_json_writer_vec_new(void)
{
	void *buf = gp_vec_new(1, 1);
	gp_json_writer *ret = malloc(sizeof(gp_json_writer));

	if (!ret || !buf) {
		free(ret);
		gp_vec_free(buf);
		return NULL;
	}

	memset(ret, 0, sizeof(*ret));

	ret->err_print = GP_JSON_ERR_PRINT;
	ret->err_print_priv = GP_JSON_ERR_PRINT_PRIV;
	ret->out = out_vec;
	ret->out_priv = buf;

	return ret;
}

void gp_json_writer_vec_free(gp_json_writer *self)
{
	if (!self)
		return;

	gp_vec_free(self->out_priv);
	free(self);
}

struct json_writer_file {
	int fd;
	size_t buf_used;
	char buf[1024];
};

static int out_writer_file_write(gp_json_writer *self, int fd, const char *buf, ssize_t buf_len)
{
	do {
		ssize_t ret = write(fd, buf, buf_len);
		if (ret <= 0) {
			err(self, "Failed to write to a file");
			return 1;
		}

		if (ret > buf_len) {
			err(self, "Wrote more bytes than requested?!");
			return 1;
		}

		buf_len -= ret;
	} while (buf_len);

	return 0;
}

static int out_writer_file(gp_json_writer *self, const char *buf, size_t buf_len)
{
	struct json_writer_file *writer_file = self->out_priv;
	size_t buf_size = sizeof(writer_file->buf);
	size_t buf_avail = buf_size - writer_file->buf_used;

	if (buf_len > buf_size/4)
		return out_writer_file_write(self, writer_file->fd, buf, buf_len);

	if (buf_len >= buf_avail) {
		if (out_writer_file_write(self, writer_file->fd,
		                          writer_file->buf, writer_file->buf_used))
			return 1;

		memcpy(writer_file->buf, buf, buf_len);
		writer_file->buf_used = buf_len;
		return 0;
	}

	memcpy(writer_file->buf + writer_file->buf_used, buf, buf_len);
	writer_file->buf_used += buf_len;

	return 0;
}

int gp_json_writer_file_close(gp_json_writer *self)
{
	struct json_writer_file *writer_file = self->out_priv;
	int saved_errno = 0;

	if (writer_file->buf_used) {
		if (out_writer_file_write(self, writer_file->fd,
		                          writer_file->buf, writer_file->buf_used))

			saved_errno = errno;
	}

	if (close(writer_file->fd)) {
		if (!saved_errno)
			saved_errno = errno;
	}

	free(self);

	if (saved_errno) {
		errno = saved_errno;
		return 1;
	}

	return 0;
}

gp_json_writer *gp_json_writer_file_open(const char *path)
{
	gp_json_writer *ret;
	struct json_writer_file *writer_file;

	ret = malloc(sizeof(gp_json_writer) + sizeof(struct json_writer_file));
	if (!ret)
		return NULL;

	writer_file = (void*)ret + sizeof(gp_json_writer);

	writer_file->fd = open(path, O_CREAT | O_WRONLY | O_TRUNC, 0664);
	if (!writer_file->fd) {
		free(ret);
		return NULL;
	}

	writer_file->buf_used = 0;

	memset(ret, 0, sizeof(*ret));

	ret->err_print = GP_JSON_ERR_PRINT;
	ret->err_print_priv = GP_JSON_ERR_PRINT_PRIV;
	ret->out = out_writer_file;
	ret->out_priv = writer_file;

	return ret;
}


