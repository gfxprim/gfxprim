#include "GP_SDL.h"
#include "GP_SDL_backend.h"
#include "GP_writepixel.h"

#define FN_NAME GP_SDL_SetPixel_8bpp
#define WRITE_PIXEL GP_WRITE_PIXEL_1BYTE
#include "generic/setpixel_generic.c"
#undef WRITE_PIXEL
#undef FN_NAME

#define FN_NAME GP_SDL_SetPixel_16bpp
#define WRITE_PIXEL GP_WRITE_PIXEL_2BYTES
#include "generic/setpixel_generic.c"
#undef WRITE_PIXEL
#undef FN_NAME

#define FN_NAME GP_SDL_SetPixel_24bpp
#define WRITE_PIXEL GP_WRITE_PIXEL_3BYTES
#include "generic/setpixel_generic.c"
#undef WRITE_PIXEL
#undef FN_NAME

#define FN_NAME GP_SDL_SetPixel_32bpp
#define WRITE_PIXEL GP_WRITE_PIXEL_4BYTES
#include "generic/setpixel_generic.c"
#undef WRITE_PIXEL
#undef FN_NAME

void GP_SDL_SetPixel(SDL_Surface *target, long color, int x, int y)
{
	int bytes_per_pixel = GP_BYTES_PER_PIXEL(target);

	/* Clip coordinates against the clip rectangle of the surface */
	int xmin, xmax, ymin, ymax;
	GP_GET_CLIP_RECT(target, xmin, xmax, ymin, ymax);
	if (x < xmin || y < ymin || x > xmax || y > ymax)
		return;

	/* Compute the address of the pixel */
	uint8_t *p = GP_PIXEL_ADDR(target, x, y);

	switch (bytes_per_pixel) {
	case 1:
		GP_WRITE_PIXEL_1BYTE(p, color);
		break;
	
	case 2:
		GP_WRITE_PIXEL_2BYTES(p, color);
		break;

	case 3:
		GP_WRITE_PIXEL_3BYTES(p, color);
		break;

	case 4:
		GP_WRITE_PIXEL_4BYTES(p, color);
		break;
	}
}

