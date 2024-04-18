#
#  gfxprim_config.py - module configuring GfxPrim code generation and
#                      known PixelTypes
#

#
# 2011      Tomas Gavenciak <gavento@ucw.cz>
# 2011-2024 Cyril Hrubis <metan@ucw.cz>
#
# This file is sourced by all the generating scripts.
# Moreover, the generated files are sourced by almost all Gfxprim sources,
# so a complete recompilation is required after any change.
#

from pixeltype import PixelType
from pixelpack import PixelPack, DB, UB
from gfxprimconfig import GfxPrimConfig

# Declared pixel sizes:
PS_1BPP_DB = PixelPack(1, bit_order=DB)
PS_1BPP_UB = PixelPack(1, bit_order=UB)
PS_2BPP_DB = PixelPack(2, bit_order=DB)
PS_2BPP_UB = PixelPack(2, bit_order=UB)
PS_4BPP_DB = PixelPack(4, bit_order=DB)
PS_4BPP_UB = PixelPack(4, bit_order=UB)
PS_8BPP = PixelPack(8)
PS_16BPP = PixelPack(16)
PS_24BPP = PixelPack(24)
PS_32BPP = PixelPack(32)

# Experimental:
PS_18BPP_DB = PixelPack(18, bit_order=DB)

config = GfxPrimConfig(
    # C name and bit-size of the GP_pixel type
    pixel_type = "uint32_t",
    pixel_size = 32,

    # List of pixel packings, explicit on purpose
    pixelpacks = [PS_1BPP_DB, PS_1BPP_UB, PS_2BPP_DB, PS_2BPP_UB, PS_4BPP_DB, PS_4BPP_UB,
                  PS_8BPP, PS_16BPP, PS_24BPP, PS_32BPP,
                  PS_18BPP_DB,
                 ],

    # List of PixelTypes, order defines the numbering.
    # The "Undefined" type is added automatically.
    pixeltypes = [

      #
      # Standard RGB types
      #
      PixelType(name='RGB101010', pixelpack=PS_32BPP, chanslist=[
      ('R', 20, 10, 2),
      ('G', 10, 10, 2),
      ('B',  0, 10, 2)]),

      PixelType(name='xRGB8888', pixelpack=PS_32BPP, chanslist=[
	  ('R', 16, 8, 2),
	  ('G',  8, 8, 2),
	  ('B',  0, 8, 2)]),

      PixelType(name='RGBA8888', pixelpack=PS_32BPP, chanslist=[
	  ('R', 24, 8, 2),
	  ('G', 16, 8, 2),
	  ('B',  8, 8, 2),
	  ('A',  0, 8, 0)]),

      PixelType(name='RGB888', pixelpack=PS_24BPP, chanslist=[
	  ('R', 16, 8, 2),
	  ('G',  8, 8, 2),
	  ('B',  0, 8, 2)]),

      PixelType(name='BGR888', pixelpack=PS_24BPP, chanslist=[
	  ('B', 16, 8, 2),
	  ('G',  8, 8, 2),
	  ('R',  0, 8, 2)]),

      PixelType(name='RGB555', pixelpack=PS_16BPP, chanslist=[
	  ('R', 10, 5, 2),
	  ('G',  5, 5, 2),
	  ('B',  0, 5, 2)]),

      PixelType(name='RGB565', pixelpack=PS_16BPP, chanslist=[
	  ('R', 11, 5, 2),
	  ('G',  5, 6, 2),
	  ('B',  0, 5, 2)]),

      PixelType(name='RGB666', pixelpack=PS_18BPP_DB, chanslist=[
	  ('R', 12, 6, 2),
	  ('G' , 6, 6, 2),
	  ('B',  0, 6, 2)]),

      PixelType(name='RGB332', pixelpack=PS_8BPP, chanslist=[
	  ('R', 5, 3, 2),
	  ('G', 2, 3, 2),
	  ('B', 0, 2, 2)]),

      #
      # CMYK
      #
      PixelType(name="CMYK8888", pixelpack=PS_32BPP, chanslist=[
	  ('K', 24, 8, 0),
	  ('Y', 16, 8, 0),
	  ('M',  8, 8, 0),
	  ('C',  0, 8, 0)]),

      #
      # Palette types
      #
      PixelType(name='P2', pixelpack=PS_2BPP_UB, chanslist=[
	  ('P', 0, 2, 0)]),

      PixelType(name='P4', pixelpack=PS_4BPP_UB, chanslist=[
	  ('P', 0, 4, 0)]),

      PixelType(name='P8', pixelpack=PS_8BPP, chanslist=[
	  ('P', 0, 8, 0)]),

      #
      # Gray-only pixel types
      #
      PixelType(name='G1_DB', pixelpack=PS_1BPP_DB, chanslist=[
	  ('V', 0, 1, 0)]),

      PixelType(name='G2_DB', pixelpack=PS_2BPP_DB, chanslist=[
	  ('V', 0, 2, 2)]),

      PixelType(name='G4_DB', pixelpack=PS_4BPP_DB, chanslist=[
	  ('V', 0, 4, 2)]),

      PixelType(name='G1_UB', pixelpack=PS_1BPP_UB, chanslist=[
	  ('V', 0, 1, 0)]),

      PixelType(name='G2_UB', pixelpack=PS_2BPP_UB, chanslist=[
	  ('V', 0, 2, 2)]),

      PixelType(name='G4_UB', pixelpack=PS_4BPP_UB, chanslist=[
	  ('V', 0, 4, 2)]),

      PixelType(name='G8', pixelpack=PS_8BPP, chanslist=[
	  ('V', 0, 8, 2)]),

      PixelType(name='GA88', pixelpack=PS_16BPP, chanslist=[
	  ('V', 0, 8, 2),
	  ('A', 8, 8, 0)]),

      PixelType(name='G16', pixelpack=PS_16BPP, chanslist=[
	  ('V', 0, 16, 2)]),
      ]
    )
