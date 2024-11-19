//SPDX-License-Identifier: LGPL-2.0-or-later
/*

   Copyright (c) 2019-2024 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @brief A proxy backend protocol definitions.
 * @file gp_proxy_proto.h
 */

#ifndef GP_PROXY_PROTO_H
#define GP_PROXY_PROTO_H

#include <core/gp_pixmap.h>
#include <core/gp_pixel.h>
#include <input/gp_event.h>
#include <stdint.h>

/**
 * @brief A proxy message type, identifies the type of the payload.
 *
 * A message type is always send in the first 32bits of the proxy message. A
 * message type is followed by a 32bit field with message lenght and optional
 * payload. The size field always includes the header size, so it's always at
 * least set to 8 bytes. Messages are also padded with zeroes so that the size
 * is always multiple of 4 bytes.
 */
enum gp_proxy_msg_types {
	/**
	 * @brief Sets an application name.
	 *
	 * This is send to the sever by the application after successfull
	 * connection.
	 *
	 * The string in the message is not NULL terminated and passed directly
	 * after the hader. The lenght of the string plus the size of the
	 * header is stored in the size field.
	 */
	GP_PROXY_NAME,
	/**
	 * @brief Requests an application exit.
	 *
	 * When application receives this message a GP_EV_SYS_QUIT event is put
	 * into the application event queue.
	 *
	 * There is no payload the message size is always 8 bytes long.
	 */
	GP_PROXY_EXIT,
	/**
	 * @brief Backend sends a basic information to the client (application).
	 *
	 * The payload lenght is sizeof(struct gp_proxy_cli_init) and among other
	 * things stores the pixel type for the application to use for drawing.
	 *
	 * This message is send to the application right after it connects to
	 * the server and the application waits for this message before it
	 * finished its initialization.
	 *
	 * See also struct gp_proxy_cli_init.
	 */
	GP_PROXY_CLI_INIT,
	/**
	 * @brief Backend sends a gp_event to the application.
	 *
	 * The payload is an input #gp_event which is put into the application
	 * event queue.
	 *
	 * See also struct gp_proxy_event.
	 */
	GP_PROXY_EVENT,
	/**
	 * @brief A buffer map request or confirmation.
	 *
	 * This message is send both by the sever and the application:
	 *
	 * - Sever asks application to map a shared buffer. Payload is struct
	 *   gp_proxy_path type.
	 *
	 * - Clients notifies server that buffer has been mapped. The message
	 *   has no payload.
	 */
	GP_PROXY_MAP,
	/**
	 * @brief A buffer unmap request or confirmation.
	 *
	 * This message is send both by the sever and the application, there is
	 * no payload in both cases:
	 *
	 * - Server asks client to unmap a shared buffer.
	 *
	 * - Client notifies sever that buffer has been unmapped.
	 */
	GP_PROXY_UNMAP,
	/**
	 * Server asks client to create a pixmap from the shared buffer.
	 *
	 * This request is send by the sever after application successfully
	 * maped a shared buffer to create a pixmap to draw to. The payload is
	 * a #gp_pixmap structure.
	 */
	GP_PROXY_PIXMAP,
	/**
	 * @brief Server requests to show application is on screen.
	 *
	 * The application will start rendering and screen updates.
	 */
	GP_PROXY_SHOW,
	/**
	 * @brief Server requests to hide application is from screen.
	 *
	 * The application will stop rendering and screen updated.
	 */
	GP_PROXY_HIDE,
	/**
	 * @brief Application asks server to update a rect on screen.
	 *
	 * This is send to a sever by the application when application rendered
	 * a rectangle into the shared memory and requests the sever to update
	 * it.
	 *
	 * The payload is the struct gp_proxy_rect.
	 */
	GP_PROXY_UPDATE,
	/**
	 * @brief Sets a cursor position.
	 *
	 * Send by the sever to an application to set the intial cursor
	 * position.
	 */
	GP_PROXY_CURSOR_POS,
	/** @brief Last message type + 1. */
	GP_PROXY_MAX,
};

/**
 * @brief A proxy input event message.
 */
struct gp_proxy_event {
	/** @brief Event type set to GP_PROXY_EVENT. */
	uint32_t type;
	/** @brief Event size set to header_size (8) + sizeof(gp_event). */
	uint32_t size;
	/** @brief The input event. */
	gp_event ev;
};

/**
 * @brief A SHM path and size for a mmap().
 */
struct gp_proxy_path {
	/** @brief A size for the mmap(). */
	size_t size;
	/** @brief A SHM path to mmap(). */
	char path[64];
};

/**
 * @brief A rectangle.
 */
struct gp_proxy_rect_ {
	uint32_t x;
	uint32_t y;
	uint32_t w;
	uint32_t h;
};

/**
 * @brief A mmap() request.
 *
 * This request is send by the sever to the application. When application
 * confirms the mmap() was finished it sends a reply without the payload.
 */
struct gp_proxy_map {
	/** @brief Event type is set to GP_PROXY_MAP. */
	uint32_t type;
	/** @brief Size is set to header size + sizeof(struct gp_proxy_path). */
	uint32_t size;
	/** @brief A proxy map payload. */
	struct gp_proxy_path map;
};

/**
 * @brief A pixmap request.
 *
 * Send by the server to create a pixmap to draw to from the mapped SHM buffer.
 */
struct gp_proxy_pixmap {
	/** @brief Event type is set to GP_PROXY_PIXMAP. */
	uint32_t type;
	/** @brief Size is set to header size + sizeof(gp_pixmap). */
	uint32_t size;
	/** @brief A pixmap payload. */
	gp_pixmap pix;
};

/**
 * @brief A rectangle update request.
 *
 * Send by the application to request update from the SHM memory to the screen.
 */
struct gp_proxy_rect {
	/** @brief Event type is set to GP_PROXY_UPDATE. */
	uint32_t type;
	/** @brief Size is set to header size + sizeof(struct gp_proxy_rect_). */
	uint32_t size;
	/** @brief The rectangle payload. */
	struct gp_proxy_rect_ rect;
};

/**
 * @brief An initial infromation send to a client (application).
 *
 * This is initial backend information needed to be send to the client (application)
 * before it can return from the gp_backend_proxy_init() function.
 */
struct gp_proxy_cli_init_ {
	/** @brief A pixel type for drawing. */
	gp_pixel_type pixel_type;
	/** @brief A display DPI. */
	unsigned int dpi;
};

/**
 * @brief An initial infromation send to a client (application).
 */
struct gp_proxy_cli_init {
	/** @brief Event type is set to GP_PROXY_CLI_INIT. */
	uint32_t type;
	/** @brief Size is set to header size + sizeof(struct gp_proxy_rect_). */
	uint32_t size;
	/** @brief The client init payload. */
	struct gp_proxy_cli_init_ cli_init;
};

/**
 * @brief A coordinate.
 */
typedef struct gp_proxy_coord {
	uint32_t x;
	uint32_t y;
} gp_proxy_coord;

/**
 * @brief A proxy cursor position.
 */
struct gp_proxy_cursor {
	/** @brief Event type set to GP_PROXY_CURSOR_POS. */
	uint32_t type;
	/** @brief Size is set to header size + sizeof(struct gp_proxy_coord). */
	uint32_t size;
	/** @brief A cursor coordinates payload. */
	struct gp_proxy_coord pos;
};

/**
 * @brief A proxy message.
 *
 * The type of payload is determined by the type field.
 */
typedef union gp_proxy_msg {
	struct {
		uint32_t type;
		uint32_t size;
		char payload[];
	};
	struct gp_proxy_event ev;
	struct gp_proxy_map map;
	struct gp_proxy_pixmap pix;
	struct gp_proxy_rect rect;
	struct gp_proxy_cli_init cli_init;
	struct gp_proxy_cursor cursor;
} gp_proxy_msg;

/**
 * @brief A proxy buffer size.
 *
 * Must be bigger than maximal message size!
 */
#define GP_PROXY_BUF_SIZE 128

/** @brief A proxy message buffer. */
typedef struct gp_proxy_buf {
	/** @brief A current buffer position. */
	size_t pos;
	/** @brief A current buffer size. */
	size_t size;
	/** @brief The buffer. */
	char buf[GP_PROXY_BUF_SIZE];
} gp_proxy_buf;

/**
 * @brief Initialize proxy buffer position and size.
 *
 * @param buf A proxy buffer.
 */
static inline void gp_proxy_buf_init(gp_proxy_buf *buf)
{
	buf->pos = 0;
	buf->size = 0;
}

/**
 * @brief Returns a string name for a proxy message type.
 *
 * @param type A proxy message type.
 *
 * @return A proxy message type name.
 */
const char *gp_proxy_msg_type_name(enum gp_proxy_msg_types type);

/**
 * @brief Parse next message in the buffer.
 *
 * Parses next message in the proxy buffer, the start of the message is stored
 * into the msg pointer. If there was no message parsed the msg pointer is set
 * to NULL.
 *
 * @param buf Proxy buffer filled by the gp_proxy_buf_recv() function.
 * @param msg Pointer to store the start of the next message to.
 *
 * @return Positive if full message was found in the buffer, zero on no or
 *         partial message and negative on error.
 */
int gp_proxy_next(gp_proxy_buf *buf, gp_proxy_msg **msg);

/**
 * @brief Receives data from from fd and stores them to the proxy buffer.
 *
 * @param fd File descriptor connected to the server/client.
 * @param buf Proxy buffer to store data to.
 * @return Number of bytes stored into the buffer, -1 on failure.
 */
int gp_proxy_buf_recv(int fd, gp_proxy_buf *buf);

/**
 * @brief Sends a message to the server/client.
 *
 * @param fd File descriptor connected to the server/client.
 * @param type Type of the message.
 * @param payload Pointer to message payload accordingly to the message type.
 *
 * @return Zero on success, non-zero on failure.
 */
int gp_proxy_send(int fd, enum gp_proxy_msg_types type, void *payload);

#endif /* GP_PROXY_PROTO_H */
