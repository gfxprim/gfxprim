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
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

 /*
  
   Based on V4L2 example code.

  */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>

#include <stdint.h>

#include "core/GP_Context.h"

#include "core/GP_Debug.h"

#include "../../config.h"

#ifdef HAVE_V4L2

#include <linux/videodev2.h>

#include "GP_Grabber.h"
#include "GP_V4L2.h"

struct v4l2_priv {
	int mode;	
	
	/* pointer to page aligned user buffer */
	void *bufptr[4];
	size_t buf_len[4];

	char device[];
};

static int v4l2_stop(struct GP_Grabber *self);

static void v4l2_exit(struct GP_Grabber *self)
{
	struct v4l2_priv *priv = GP_GRABBER_PRIV(self);
	int i;

	GP_DEBUG(1, "Grabber '%s' exitting", priv->device);
	
	v4l2_stop(self);
	
	if (priv->mode == 2) {
		for (i = 0; i < 4; i++)
			munmap(priv->bufptr[i], priv->buf_len[i]);
	}

	close(self->fd);
	GP_ContextFree(self->frame);
	free(self);
}

#define CLAMP(x, max)    \
	if (x > max)     \
		x = max; \
	if (x < 0)       \
		x = 0;   \

#define MUL 1024

static void v4l2_yuv422_fillframe(struct GP_Grabber *self, void *buf)
{
	unsigned int i, j;
	unsigned char *py, *pu, *pv;
	unsigned char *tmp = self->frame->pixels;

	py = buf;
	pu = buf + 1;
	pv = buf + 3;

	for (i = 0; i < self->frame->h; i++) {
		for (j = 0; j < self->frame->w; j++) {
			int32_t PU = ((int32_t)*pu) - 128;
			int32_t PV = ((int32_t)*pv) - 128;

			int32_t R = MUL * (*py) + ((int32_t)(MUL * 1.772)) * PU;
			int32_t G = MUL * (*py) - ((int32_t)(MUL * 0.344)) * PU
			            - ((int32_t)(MUL * 0.714)) * PV;
			int32_t B = MUL * (*py) + ((int32_t)(MUL * 1.402)) * PV;
			
			R = (R + MUL/2)/MUL;
			G = (G + MUL/2)/MUL;
			B = (B + MUL/2)/MUL;

			CLAMP(R, 255);
			CLAMP(G, 255);
			CLAMP(B, 255);
			
			*tmp++ = R;
			*tmp++ = G;
		  	*tmp++ = B;

			py += 2;
			
			if ((j & 1) == 1) {
				pu += 4;
				pv += 4;
			}
		}
	}
}

static int v4l2_poll(struct GP_Grabber *self)
{
	struct v4l2_priv *priv = GP_GRABBER_PRIV(self);
	
	GP_DEBUG(3, "Grabber '%s' poll", priv->device);
	
	/* read/write interface */
	if (priv->mode == 1) {
		GP_WARN("Read/write I/O not implemented.");
		return 0;
	}

	/* mmaped interface */
	struct v4l2_buffer buf;

	memset(&buf, 0, sizeof(buf));

	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;

	if (ioctl(self->fd, VIDIOC_DQBUF, &buf)) {
		switch (errno) {
		case EAGAIN:
			return 0;
        	default:
			GP_WARN("Failed to ioctl VIDIOC_DQBUF on '%s' : %s",
			        priv->device, strerror(errno));
			return 0;
		}
	}

	if (buf.index >= 4) {
		GP_WARN("Got invalid buffer index on '%s'", priv->device);
		return 0;
	}

	v4l2_yuv422_fillframe(self, priv->bufptr[buf.index]);

	if (ioctl(self->fd, VIDIOC_QBUF, &buf)) {
		GP_WARN("Failed to ioctl VIDIOC_QBUF on '%s' : %s",
		        priv->device, strerror(errno));
	}

	return 1;
}

static int v4l2_start(struct GP_Grabber *self)
{
	struct v4l2_priv *priv = GP_GRABBER_PRIV(self);

	/* read/write interface */
	if (priv->mode == 1)
		return 0;

	/* mmap interface */
	int i;
	struct v4l2_buffer buf;
	memset(&buf, 0, sizeof(buf));
	
	for (i = 0; i < 4; i++) {
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;
	
		if (ioctl(self->fd, VIDIOC_QBUF, &buf)) {
			GP_WARN("Failed to ioclt VIDIOC_QBUF on '%s': %s",
			        priv->device, strerror(errno));
			return 1;
		}
	}

	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (ioctl(self->fd, VIDIOC_STREAMON, &type)) {
		GP_WARN("Failed to ioclt VIDIOC_STREAMON on '%s': %s",
		        priv->device, strerror(errno));
		return 1;
	}

	return 0;
}

static int v4l2_stop(struct GP_Grabber *self)
{
	struct v4l2_priv *priv = GP_GRABBER_PRIV(self);

	/* read/write interface */
	if (priv->mode == 1)
		return 0;

	/* mmap interface */
	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (ioctl(self->fd, VIDIOC_STREAMOFF, &type)) {
		GP_WARN("Failed to ioclt VIDIOC_STREAMON on '%s': %s",
		        priv->device, strerror(errno));
		return 1;
	}

	return 0;
}

struct GP_Grabber *GP_GrabberV4L2Init(const char *device,
                                      unsigned int preferred_width,
				      unsigned int preferred_height)
{
	int fd, i, err;
	int mode = 0;

	GP_DEBUG(1, "Opening V4L2 grabber '%s'", device);

	fd = open(device, O_RDWR | O_NONBLOCK);

	if (fd < 0) {
		err = errno;
		GP_WARN("Failed to open V4L2 grabber '%s'", device);
		goto err;
	}
	
	struct v4l2_capability cap;
	
	if (ioctl(fd, VIDIOC_QUERYCAP, &cap)) {
		err = errno;
		GP_WARN("ioctl VIDIOC_QUERYCAP failed, '%s' not V4L2 device?",
		        device);
		goto err0;
	}

	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
		err = ENOSYS;
		GP_WARN("Device '%s' has no capture capability", device);
		goto err0;
	}


	if (!(cap.capabilities & V4L2_CAP_READWRITE)) {
		GP_DEBUG(1, "Device '%s' doesn't support read write I/O", device);
	} else {
		mode = 1;
	}

	if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
		GP_DEBUG(1, "Device '%s' doesn't support streaming I/O", device);
	} else {
		mode = 2;
	}

	if (mode == 0) {
		err = ENOSYS;
		GP_WARN("No suitable mode found for '%s'", device);
		goto err0;
	}

	struct v4l2_cropcap cropcap;
	struct v4l2_crop crop;

	memset(&cropcap, 0, sizeof(cropcap));

	cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (ioctl(fd, VIDIOC_CROPCAP, &cropcap) == 0) {
		crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		/* reset to default */
		crop.c = cropcap.defrect;
		
		if (ioctl(fd, VIDIOC_S_CROP, &crop)) {
			/* error/cropping not supported */
		}

	} else {
		/* errors ignored? */
	}

	struct v4l2_format fmt;
	
	memset(&fmt, 0, sizeof(fmt));

	fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width       = preferred_width;
	fmt.fmt.pix.height      = preferred_height;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
	fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;

	if (ioctl(fd, VIDIOC_S_FMT, &fmt)) {
		err = errno;
		GP_WARN("Failed to set video format for device '%s'", device);
		goto err0;
	}

	GP_Grabber *new = malloc(sizeof(GP_Grabber) + sizeof(struct v4l2_priv) + strlen(device) + 1);

	if (new == NULL) {
		err = ENOMEM;
		GP_WARN("Malloc failed :(");
		goto err0;
	}

	new->frame = GP_ContextAlloc(fmt.fmt.pix.width, fmt.fmt.pix.height, GP_PIXEL_RGB888);
	
	if (new->frame == NULL) {
		err = ENOMEM;
		goto err1;
	}

	struct v4l2_priv *priv = GP_GRABBER_PRIV(new);

	strcpy(priv->device, device);
	priv->mode = mode;
	
	switch (mode) {
	case 1:
	break;
	case 2: {
		/* setup mmap interface */
		struct v4l2_requestbuffers req;

		memset(&req, 0, sizeof(req));

		req.count  = 4;
		req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		req.memory = V4L2_MEMORY_MMAP;
		
		if (ioctl(fd, VIDIOC_REQBUFS, &req)) {
			err = errno;
			GP_WARN("Failed to ioctl VIDIOC_REQBUFS on '%s' : %s",
			        device, strerror(errno));
			goto err2;
		}

		if (req.count != 4) {
			err = ENOSYS;
			GP_WARN("Unexpected number of buffers on '%s'", device);
			goto err2;
		}
		
		struct v4l2_buffer buf;
		memset(&buf, 0, sizeof(buf));

		priv->bufptr[0] = NULL;
		priv->bufptr[1] = NULL;
		priv->bufptr[2] = NULL;
		priv->bufptr[3] = NULL;

		for (i = 0; i < 4; i++) {
			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.memory = V4L2_MEMORY_MMAP;
			buf.index = i;

			if (ioctl(fd, VIDIOC_QUERYBUF, &buf)) {
				err = errno;
				GP_WARN("Failed to ioclt VIDIOC_QUERYBUF on '%s': %s",
				        device, strerror(errno));
				goto err2;
			}

			priv->bufptr[i] = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
			priv->buf_len[i] = buf.length;	

			if (priv->bufptr[i] == MAP_FAILED) {
				err = errno;
				GP_WARN("mmap failed on '%s': %s", device, strerror(errno));
				goto err3;
			}
		}
	} break;
	}

	new->fd = fd;
	new->Exit = v4l2_exit;
	new->Poll = v4l2_poll;
	new->Start = v4l2_start;
	new->Stop = v4l2_stop;

	return new;
err3:
	for (i = 0; i < 4; i++)
		if (priv->bufptr[i] != NULL)
			munmap(priv->bufptr[i], priv->buf_len[i]);
err2:
	GP_ContextFree(new->frame);
err1:
	free(new);
err0:
	close(fd);
err:
	errno = err;
	return NULL;
}

#else

struct GP_Grabber *GP_GrabberV4L2Init(const char GP_UNUSED(*device),
                                      unsigned int GP_UNUSED(preferred_width),
				      unsigned int GP_UNUSED(preferred_height))
{
	GP_WARN("V4L2 support not compiled in.");

	errno = ENOSYS;
	return NULL;
}

#endif /* HAVE_V4L2 */
