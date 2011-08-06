#
# gfxprim.render_utils
#

import jinja2
import logging as log
import os
import time
import re

def create_environment(config, template_dir):
  env = jinja2.Environment(
      line_statement_prefix = "%%",
      undefined = jinja2.StrictUndefined,
      loader = jinja2.FileSystemLoader(template_dir))
  env.globals['undefined'] = jinja2.StrictUndefined()
  env.globals['pixelsizes'] = config.pixelsizes
  env.globals['pixeltypes'] = config.pixeltypes
  env.globals['gfxprim_config'] = config
  return env


def render_file(source, result, config, template_dir):
  env = create_environment(config, template_dir)
  with open(source) as source_file:
    source_text = source_file.read()
  # Hack to preserve empty lines before %% line_statement
  source_text = re.sub("\n\n[ \t]*%%", "\n{{''}}\n%%", source_text)

  tmpl = env.from_string(source_text)
  result_text = tmpl.render(
      date = time.ctime(),
      target = result,
      template = source,
      header_guard = \
        os.path.split(result)[1].upper().replace('.', '_').replace('-', '_'),
      )
  with open(result, "w") as result_file:
    result_file.write(result_text)


def load_gfxprimconfig(config_file = None):
  """Initialize GfxPrimConfig from a given or guessed config file.
  Looks for the file by parameter, in env['PIXELTYPE_DEFS'] and
  in dir(__file__)/../../gfxprim_config.py, in that order.

  Returns GfxPrimConfig or None on error
  """

  if not config_file:
    config_file = os.environ.get("PIXELTYPE_DEFS", None)
  if not config_file:
    path = os.path.dirname(os.path.abspath(__file__))
    config_file = os.path.abspath(
        os.path.join(path, "..", "..", "gfxprim_config.py"))
  if not os.path.exists(config_file):
    log.error("WARNING: GfxPrimConfig file %s not found!\n",
        config_file)
    return None

  from gfxprim.pixeltype import PixelType
  from gfxprim.pixelsize import PixelSize, LE, BE
  from gfxprim.gfxprimconfig import GfxPrimConfig
  l = {"PixelType": PixelType,
       "PixelSize": PixelSize,
       "LE": LE,
       "BE": BE,
       "GfxPrimConfig": GfxPrimConfig
       }
  execfile(config_file, globals(), l)
  config = l["config"]
  return config

