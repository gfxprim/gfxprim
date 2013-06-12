#
# gfxprim.render_utils
#

import logging as log
import os
import time
import re
from math import floor, ceil

# Custom filters

def comment(lines):
  """Adds space and asterisk before each line from lines"""
  ret = ''
  l = lines.split('\n')

  # Remove leading empty line
  if l[0] == '':
    l = l[1:]

  # Remove traling empty line
  if l[-1] == '':
    l.pop()

  for i in l:
    ret += ' * ' + i + '\n'

  return ret

try:
  import jinja2
except:
  log.fatal("Jinja 2 is required to compile gfxprim.")

def template_error(s, *args):
  raise Exception(s, *args)

def create_environment(config, template_dir):
  env = jinja2.Environment(
      line_statement_prefix = "%%",
      undefined = jinja2.StrictUndefined,
      loader = jinja2.FileSystemLoader(template_dir))
  env.globals['undefined'] = jinja2.StrictUndefined()
  env.globals['pixelsizes'] = config.pixelsizes
  env.globals['pixelsizes_by_bpp'] = config.pixelsizes_by_bpp
  env.globals['pixeltypes'] = config.pixeltypes
  env.globals['pixeltypes_dict'] = config.pixeltypes_dict
  env.globals['config'] = config
  from .pixelsize import LE, BE
  env.globals['LE'] = LE
  env.globals['BE'] = BE
  env.globals['len'] = len
  env.globals['error'] = template_error
  env.globals['hex'] = lambda x: hex(x).rstrip('L')
  # Propagate some python buildins
  env.globals['int'] = int
  env.globals['float'] = float
  env.globals['round'] = round
  env.globals['floor'] = floor
  env.globals['ceil'] = ceil
  env.globals['min'] = min
  env.globals['max'] = max
  # Add custom filters
  env.filters['comment'] = comment
  return env


def render_file(env, source, result):
  source_file = open(source)
  try:
    source_text = source_file.read()
  finally:
    source_file.close()
  # Hack to preserve empty lines before %% line_statement
  source_text = re.sub("\n\n[ \t]*%%", "\n{{''}}\n%%", source_text)

  tmpl = env.from_string(source_text)
  tmpl.filename = source
  result_text = tmpl.render(
      date = time.ctime(),
      target = result,
      template = source,
      header_guard = \
        os.path.split(result)[1].upper().replace('.', '_').replace('-', '_'),
      )
  result_file = open(result, "w")
  try:
    result_file.write(result_text)
  finally:
    result_file.close()

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

  from .pixeltype import PixelType
  from .pixelsize import PixelSize, LE, BE
  from .gfxprimconfig import GfxPrimConfig
  l = {"PixelType": PixelType,
       "PixelSize": PixelSize,
       "LE": LE,
       "BE": BE,
       "GfxPrimConfig": GfxPrimConfig
       }
  # python 3.2 doesn't have execfile, curse you python devs
  # execfile(config_file, globals(), l)
  cf = open(config_file)
  exec(compile(cf.read(), config_file, 'exec'), globals(), l)
  cf.close()

  config = l["config"]
  return config

