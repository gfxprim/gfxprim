// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2021 Cyril Hrubis <metan@ucw.cz>
 */

/*

  STB Image support for GFXPrim

*/

#include "../../config.h"

#include <errno.h>

#include <loaders/gp_io.h>
#include <loaders/gp_line_convert.h>
#include <loaders/gp_loaders.gen.h>

#include <core/gp_debug.h>
#include <core/gp_common.h>

#ifdef HAVE_STB_IMAGE

/* STB Image Configuration */
#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO
#define STBI_NO_THREAD_LOCALS
#define STBI_NO_LINEAR
#define STBI_NO_HDR
#define STBI_ASSERT(x) GP_ASSERT(x, "stb_image.h assertion")
#include <loaders/stb/stb_image.h>

int gp_match_stb_image(const void *buf) {
    int x;
    int y;
    int comp;

    if (stbi_info_from_memory(buf, sizeof buf, &x, &y, &comp) == 1) {
        return 0;
    }

    return 1;
}

int gp_read_stb_image_ex(gp_io *io, gp_pixmap **img, gp_storage *storage,
                   gp_progress_cb *callback) {
    // Allocate the buffer.
    off_t size = gp_io_size(io);
    void* buffer = malloc((size_t)size);
    if (!buffer) {
        GP_DEBUG(1, "Failed to allocate memory");
		errno = ENOMEM;
        return 1;
    }

    // Read to the buffer.
    if (gp_io_read(io, buffer, (size_t)size) <= 0) {
        GP_DEBUG(1, "Failed to read IO");
		errno = EIO;
        return 1;
    }

    // Load the image data.
    int width;
    int height;
    int components;
    stbi_uc* data = stbi_load_from_memory(buffer, (int)size, &width, &height, &components, (*img)->bpp);
    if (!data) {
        GP_DEBUG(1, "Failed to read STB Image");
		errno = EIO;
        return 1;
    }

    // Ensure the destination has the correct image size.
    if (gp_pixmap_resize(*img, width, height) != 0) {
        free(data);
        GP_DEBUG(1, "Failed to resize image to correct STB Image size");
		errno = ENOSYS;
        return 1;
    }

    // Set the pixmap data.
    free((*img)->pixels);
    (*img)->pixels = data;

    // Finish up.
    gp_progress_cb_done(callback);

    return 0;
}

#endif /* HAVE_STB_IMAGE */

const gp_loader gp_stb_image = {
#ifdef HAVE_STB_IMAGE
    .read = gp_read_stb_image_ex,
    .match = gp_match_stb_image,
#endif

    .fmt_name = "STB Image",
    .extensions = {
#ifdef HAVE_STB_IMAGE
#ifndef STBI_NO_JPEG
        "jpg",
        "jpeg",
#endif
#ifndef STBI_NO_PNG
        "png",
#endif
#ifndef STBI_NO_BMP
        "bmp",
#endif
#ifndef STBI_NO_PSD
        "psd",
#endif
#ifndef STBI_NO_TGA
        "tga",
#endif
#ifndef STBI_NO_GIF
        "gif",
#endif
#ifndef STBI_NO_HDR
        "hdr",
#endif
#ifndef STBI_NO_PIC
        "pic",
#endif
#ifndef STBI_NO_PNM
        "pnm",
        "pgm",
#endif
#endif
        NULL
    },
};