#!/usr/bin/python
#
# Main source file generating script
# Collects all known file generators from the generators/ directory
#
# 2011 - Tomas Gavenciak <gavento@ucw.cz>
#

from gfxprim import render_utils
import jinja2
import logging as log
from optparse import OptionParser


parser = OptionParser(usage="usage: %prog [options] <template> <output>")
parser.add_option("-t", "--templates", dest="templates",
                  help="Directory with templates.", default=".")
parser.add_option("-c", "--config", dest="config",
                  help="GfxPrim config file.", default=None)

def main(options, args):
  config = render_utils.load_gfxprimconfig(options.config)
  assert config
  log.info("Rendering template %s to %s", args[0], args[1])
  env = render_utils.create_environment(config, options.templates)
  render_utils.render_file(env, args[0], args[1])


if __name__ == '__main__':
  log.debug("Jinja version %s", jinja2.__version__)
  (options, args) = parser.parse_args()
  if len(args) != 2:
    parser.error()
  main(options, args)
