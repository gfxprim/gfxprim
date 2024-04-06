// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2023 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_pixel_alias.h
 * @brief Pixel type aliases.
 */
#ifndef CORE_GP_PIXEL_ALIAS_H
#define CORE_GP_PIXEL_ALIAS_H

#include <core/gp_pixel.gen.h>

/**
 * @brief List of pixel type aliases.
 *
 * Defines the more common grayscal pixel packings as default.
 */
enum gp_pixel_type_alias {
#ifdef GP_PIXEL_G1_UB
	/** @brief Default packing for G1 pixel is UB */
	GP_PIXEL_G1 = GP_PIXEL_G1_UB,
#endif
#ifdef GP_PIXEL_G2_UB
	/** @brief Default packing for G2 pixel is UB */
	GP_PIXEL_G2 = GP_PIXEL_G2_UB,
#endif
#ifdef GP_PIXEL_G4_UB
	/** @brief Default packing for G4 pixel is UB */
	GP_PIXEL_G4 = GP_PIXEL_G4_UB,
#endif
};

#ifdef GP_PIXEL_G1_UB
# define GP_PIXEL_G1 GP_PIXEL_G1_UB
# define GP_PIXEL_ADDR_1BPP GP_PIXEL_ADDR_1BPP_UB
# define GP_PIXEL_ADDR_OFFSET_1BPP GP_PIXEL_ADDR_OFFSET_1BPP_UB
# define gp_getpixel_raw_1BPP gp_getpixel_raw_1BPP_UB
# define gp_putpixel_raw_1BPP gp_putpixel_raw_1BPP_UB
#endif

#ifdef GP_PIXEL_G2_UB
# define GP_PIXEL_G2 GP_PIXEL_G2_UB
# define GP_PIXEL_ADDR_2BPP GP_PIXEL_ADDR_2BPP_UB
# define GP_PIXEL_ADDR_OFFSET_2BPP GP_PIXEL_ADDR_OFFSET_2BPP_UB
# define gp_getpixel_raw_2BPP gp_getpixel_raw_2BPP_UB
# define gp_putpixel_raw_2BPP gp_putpixel_raw_2BPP_UB
#endif

#ifdef GP_PIXEL_G4_UB
# define GP_PIXEL_G4 GP_PIXEL_G4_UB
# define GP_PIXEL_ADDR_4BPP GP_PIXEL_ADDR_4BPP_UB
# define GP_PIXEL_ADDR_OFFSET_4BPP GP_PIXEL_ADDR_OFFSET_4BPP_UB
# define gp_getpixel_raw_4BPP gp_getpixel_raw_4BPP_UB
# define gp_putpixel_raw_4BPP gp_putpixel_raw_4BPP_UB
#endif

#endif /* CORE_GP_PIXEL_ALIAS_H */
