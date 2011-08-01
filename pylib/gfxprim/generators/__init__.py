#
#  gfxprim.generate - global config and its loading
#

import os
import sys

# Global config instance

config = None


def load_gfxprim_config(config_file = None):
  """Initialize GfxPrimConfig from a given or guessed config file.
  Looks for the file by parameter, in env['PIXELTYPE_DEFS'] and
  in dir(__file__)/../../../gfxprim_config.py, in that order.
  """

  if not config_file:
    config_file = os.environ.get("PIXELTYPE_DEFS", None)
  if not config_file:
    path = os.path.dirname(os.path.abspath(__file__))
    config_file = os.path.abspath(
	os.path.join(path, "..", "..", "..", "gfxprim_config.py"))
  if not os.path.exists(config_file):
    sys.stderr.write("WARNING: GfxPrimConfig file %s not found!\n" %
	config_file)

  global config
  assert not config
  from gfxprim.generators.pixeltype import PixelType
  from gfxprim.generators.gfxprimconfig import GfxPrimConfig
  l = {"PixelType": PixelType, "GfxPrimConfig": GfxPrimConfig}
  execfile(config_file, globals(), l)
  config = l["config"]
  assert config
