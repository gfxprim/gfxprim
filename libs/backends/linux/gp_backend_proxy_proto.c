//SPDX-License-Identifier: LGPL-2.0-or-later
/*

   Copyright (c) 2019-2025 Cyril Hrubis <metan@ucw.cz>

 */

#include <core/gp_debug.h>
#include <inttypes.h>
#include <string.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <errno.h>
#include <backends/gp_proxy_proto.h>

static int validate_msg(const union gp_proxy_msg *msg, unsigned int size)
{
	if (size < 8)
		return 0;

	if (msg->type >= GP_PROXY_MAX) {
		GP_WARN("Invalid message type %" PRIu32, msg->type);
		return -1;
	}

	if (size < msg->size)
		return 0;

	return msg->size;
}

int gp_proxy_buf_recv(int fd, struct gp_proxy_buf *buf, int block)
{
	ssize_t ret;
	char *bufp = buf->buf + buf->pos + buf->size;
	size_t len = GP_PROXY_BUF_SIZE - buf->pos - buf->size;

	ret = recv(fd, bufp, len, block ? 0 : MSG_DONTWAIT);
	if (ret > 0)
		buf->size += ret;

	return ret;
}

int gp_proxy_next(struct gp_proxy_buf *buf, union gp_proxy_msg **msg)
{
	int msg_size;
	union gp_proxy_msg *ret = (void*)buf->buf + buf->pos;

	*msg = NULL;

	msg_size = validate_msg(ret, buf->size);

	if (!msg_size) {
		if (buf->size)
			memmove(buf->buf, buf->buf + buf->pos, buf->size);
		buf->pos = 0;
		return 0;
	}

	if (msg_size < 0)
		return -1;

	buf->pos += msg_size;
	buf->size -= msg_size;

	*msg = ret;

	return 1;
}

const char *gp_proxy_msg_type_name(enum gp_proxy_msg_types type)
{
	switch (type) {
	case GP_PROXY_NAME:
		return "GP_PROXY_NAME";
	case GP_PROXY_EXIT:
		return "GP_PROXY_EXIT";
	case GP_PROXY_CLI_INIT:
		return "GP_PROXY_CLI_INIT";
	case GP_PROXY_EVENT:
		return "GP_PROXY_EVENT";
	case GP_PROXY_MAP:
		return "GP_PROXY_MAP";
	case GP_PROXY_UNMAP:
		return "GP_PROXY_UNMAP";
	case GP_PROXY_PIXMAP:
		return "GP_PROXY_PIXMAP";
	case GP_PROXY_UPDATE:
		return "GP_PROXY_UPDATE";
	case GP_PROXY_SHOW:
		return "GP_PROXY_SHOW";
	case GP_PROXY_HIDE:
		return "GP_PROXY_HIDE";
	case GP_PROXY_CURSOR_POS:
		return "GP_PROXY_CURSOR_POS";
	case GP_PROXY_MAX:
	break;
	}

	return "???";
}

int gp_proxy_send(int fd, enum gp_proxy_msg_types type, void *payload)
{
	union gp_proxy_msg msg;
	size_t payload_size = 0;

	msg.type = type;
	msg.size = 8;

	GP_DEBUG(3, "Sending type %s (%i)", gp_proxy_msg_type_name(type), type);

	switch (type) {
	case GP_PROXY_NAME:
		payload_size = strlen(payload);
	break;
	case GP_PROXY_CLI_INIT:
		payload_size += sizeof(struct gp_proxy_cli_init_);
	break;
	case GP_PROXY_EVENT:
		payload_size = sizeof(gp_event);
	break;
	case GP_PROXY_MAP:
		if (payload)
			payload_size = sizeof(struct gp_proxy_path);
	break;
	case GP_PROXY_PIXMAP:
		payload_size = sizeof(gp_pixmap);
	break;
	case GP_PROXY_UPDATE:
		payload_size = sizeof(struct gp_proxy_rect);
	break;
	case GP_PROXY_CURSOR_POS:
		payload_size = sizeof(struct gp_proxy_coord);
	break;
	default:
	break;
	}

	msg.size += payload_size;

	size_t padd_size = payload_size % 4 ? 4 - payload_size % 4 : 0;
	char padd[3] = {0, 0, 0};

	msg.size += padd_size;

	struct iovec vec[] = {
		{.iov_base = &msg, .iov_len = 8},
		{.iov_base = payload, .iov_len = payload_size},
		{.iov_base = padd, .iov_len = padd_size},
	};

	struct msghdr hdr = {
		.msg_iov = vec,
		.msg_iovlen = 3,
	};

	ssize_t ret = sendmsg(fd, &hdr, 0);

	if (ret < 0) {
		GP_WARN("sendmsg(): %s", strerror(errno));
		return 1;
	}

	if ((uint32_t)ret != msg.size) {
		GP_WARN("sendmsg() returned %zi != %" PRIu32, ret, msg.size);
		return 1;
	}

	return 0;
}
