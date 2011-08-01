#
#  gfxprim_config.py - module configuring GfxPrim code generation and
#                      known PixelTypes
# 

#
# 2011 - Tomas Gavenciak <gavento@ucw.cz>
# 2011 - Cyril Hrubis <metan@ucw.cz>
#
# This file is sourced by all the generating scripts.
# Moreover, the generated files are sourced by almost all Gfxprim sources,
# so a complete recompilation is required after any change. 
#

config = GfxPrimConfig(

    # C name and bit-size of the GP_pixel type
    pixel_type = "uint32_t",
    pixel_size = 32,

    # List of pixel sizes (bpp), explicit on purpose
    sizes = [1, 2, 4, 8, 16, 32]

    # bit endians to generate, keep this fixed to LE, BE for now
    bit_endians = ['LE', 'BE']

    # List of PixelTypes, order defines the numbering. 
    # The "Undefined" type is added automatically.
    types = [

      #
      # Standard RGB types
      #

      PixelType(name='RGBx8888', size=32, chanslist=[
	('R', 16, 8),
	('G', 8, 8),
	('B', 0, 8)]),

      PixelType(name='RGBA8888', size=32, chanslist=[
	  ('R', 24, 8),
	  ('G', 16, 8),
	  ('B', 8, 8),
	  ('A', 0, 8)]),

      PixelType(name='RGB888', size=24, chanslist=[
	  ('R', 16, 8),
	  ('G', 8, 8),
	  ('B', 0, 8)]),

      PixelType(name='RGB565', size=16, chanslist=[
	  ('R', 11, 5),
	  ('G', 5, 6),
	  ('B', 0, 5)]),

      #
      # Palette types
      #

      PixelType(name='P2', size=2, bit_endian='LE', chanslist=[
	  ('P', 0, 2)]),

      PixelType(name='P4', size=4, bit_endian='LE', chanslist=[
	  ('P', 0, 4)]),

      PixelType(name='P8', size=8, bit_endian='LE', chanslist=[
	  ('P', 0, 8)]),

      #
      # Gray-only pixel types
      #

      PixelType(name='V1', size=1, bit_endian='LE', chanslist=[
	  ('V', 0, 1)]),

      PixelType(name='V2', size=2, bit_endian='LE', chanslist=[
	  ('V', 0, 2)]),

      PixelType(name='V4', size=4, bit_endian='LE', chanslist=[
	  ('V', 0, 4)]),

      PixelType(name='V8', size=8, bit_endian='LE', chanslist=[
	  ('V', 0, 8)]),

      #
      # Experiments
      #

      PixelType(name='VA12', size=4, bit_endian='BE', chanslist=[
	  ('A', 1, 2),
	  ('V', 3, 1)]),
      ]
    )
