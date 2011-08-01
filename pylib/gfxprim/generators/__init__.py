#
#  gfxprim.generate
#


# Global config instance

config = None


def load_gfxprim_config(config_file = None):
  """Initialize GfxPrimConfig from a given or guessed config file.
  Looks for the file by parameter, in env['PIXELTYPE_DEFS'] and 
  in dir(__file__)/../../../gfxprim_config.py, in that order.
  """
  if not defs_file:
    defs_file = os.environ.get("PIXELTYPE_DEFS", None)
  if not defs_file:
    path = os.path.dirname(os.path.abspath(__file__))
    defs_file = os.path.abspath(
	os.path.join(path, "..", "..", "..", "gfxprim_config.py"))
  if not os.path.exists(defs_file):
    sys.stderr.write("WARNING: GfxPrimConfig file %s not found!\n" % defs_file)

  from gfxprim.generate.pixeltype import PixelType 
  from gfxprim.generate.gfxprimconfig import GfxPrimConfig
  l = {"PixelType": PixelType, "GfxPrimConfig": GfxPrimConfig}
  execfile(defs_file, globals(), l)

  config = l["config"]
  assert config
