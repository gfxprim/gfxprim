#
#  gfxprim.generators.utils - Module with templates and tools for writing
#                             generated code, especially C source and headers.
#
# 2011 - Tomas Gavenciak <gavento@ucw.cz>
#


import jinja2
import logging as log

def j2render(tmpl, **kw):
  "Internal helper to render jinja2 templates (with StrictUndefined)"
  t2 = tmpl.rstrip('\n') # Jinja strips the last '\n', so add these later
  return jinja2.Template(t2, undefined=jinja2.StrictUndefined).render(**kw) + \
      tmpl[len(t2):]


def load_generators():
  "Load all modules containig generators to allow them to register"
  # TODO: write proper discovery
  import gfxprim.generators.make_collected_tests
  import gfxprim.generators.core.make_GP_Pixel
  import gfxprim.generators.core.make_GP_Blit
  import gfxprim.generators.core.make_GP_Convert


def generate_file(fname):
  "Function trying to generate file `fname` using matching known generator."
  from gfxprim.generators.generator import known_generators
  matches = []
  for k in known_generators:
    if k.matches_path(fname):
      matches.append(k)
  if len(matches) == 0:
    log.fatal("No known generators match '%s'" % (fname,))
  if len(matches) > 1:
    log.fatal("Multiple generators match '%s': %s" % (fname, str(matches)))
  s = matches[0].generate()
  with open(fname, "wt") as f:
    f.write(s)


def hmask(bits):
  "Helper returning hex mask for that many bits"
  return hex((1<<bits)-1).rstrip('L')
