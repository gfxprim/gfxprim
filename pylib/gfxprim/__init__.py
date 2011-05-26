# GfxPrim auxiliary libraries
# Contains C code-generation
#
# 2011 - Tomas Gavenciak <gavento@ucw.cz> 
#

import sys

def die(msg):
  "Exit(1) with a message."
  if not msg.endswith('\n'): 
    msg += '\n'
  sys.stderr.write(msg)
  sys.exit(1)


