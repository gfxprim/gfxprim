#!/usr/bin/python
#
# Script generating collected_tests.gen.c
#
# 2011 - Tomas Gavenciak <gavento@ucw.cz>
#

from gp_codegen import render_utils, test_collection
import jinja2
import logging as log
from optparse import OptionParser

template_file = "collected_tests.c.t"

parser = OptionParser(usage="usage: %prog [options] <scan_dir> <output_file>")
parser.add_option("-t", "--templates", dest="templates",
                  help="Directory with templates.", default=".")
parser.add_option("-c", "--config", dest="config",
                  help="GfxPrim config file.", default=None)

def main(options, args):
  config = render_utils.load_gfxprimconfig(options.config)
  assert config
  log.info("Scanning dir %s for tests, generating %s", args[0], args[1])
  env = render_utils.create_environment(config, options.templates)
  env.globals['suites'] = test_collection.collect_suites(args[0])
  render_utils.render_file(env, options.templates + '/' + template_file, args[1])


if __name__ == '__main__':
  log.debug("Jinja version %s", jinja2.__version__)
  (options, args) = parser.parse_args()
  if len(args) != 2:
    parser.error()
  main(options, args)
