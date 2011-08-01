#!/usr/bin/python
#
# Main source file generating script
# Collects all known file generators from the generators/ directory
#
# 2011 - Tomas Gavenciak <gavento@ucw.cz>
#

import sys
from gfxprim.generators import utils
from gfxprim import die

usage = """Usage: %s [files_to_generate...]
The files are matched only based on path suffixes, but written to given paths.
"""

def main():
  utils.load_generators()
  if len(sys.argv) <= 1:
    die(usage)
  for f in sys.argv[1:]:
    utils.generate_file(f)

if __name__ == '__main__':
  main()
