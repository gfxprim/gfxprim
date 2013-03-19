#!/usr/bin/env python
import gfxprim.core as core
import gfxprim.backends as backends
import gfxprim.loaders as loaders
import gfxprim.gfx as gfx
import random
from math import sqrt

AA = True
W = 320*2
H = 240*2
BPP = 24
N = 8 # Try 5 or 42

# Gravity, brake factor and desired total speeds (in dir. X and total)
G = 0.1  # Adjust: 0.1 for N<20, 0.01 for N=100
BRAKE = 0.001
VX0 = 0.6
V0 = 0.9

# Try one max and other ~50 :
MAXR = 255
MAXG = 255
MAXB = 255

# timeout in moves, -1 for none
TIMEOUT = -1
# after timeout save to this file, None for none
SAVETO = "gravplot.png"

class elem(object):
  def __init__(self):
    #self.x = random.uniform(0.0 * W, 0.6 * W)
    self.x = 0
    self.y = random.uniform(0.2 * H, 0.8 * H)
    self.vx = random.uniform(VX0 - 0.5, VX0 + 0.5)
    self.vy = random.uniform(-0.5, 0.5)
    self.r = random.randint(MAXR / 5, MAXR)
    self.g = random.randint(MAXG / 5, MAXG)
    self.b = random.randint(MAXB / 5, MAXB)

  def grav(self, other, t):
    dx = other.x - self.x
    dy = other.y - self.y
    # Minimal distance is bounded
    d2 = max(dx ** 2 + dy ** 2, 0.1 ** 2)
    f = G / d2
    self.vx += dx * f * t
    self.vy += dy * f * t

  def move(self, t):
    self.x += self.vx * t
    self.y += self.vy * t
    # "Gravity" to y=H/2
    if self.y > 0.6 * H:
      self.vy -= V0 * t * BRAKE
    if self.y < 0.4 * H:
      self.vy += V0 * t * BRAKE
    # Magic to adjust speed
    v = sqrt(self.vx ** 2 + self.vy ** 2)
    self.vx *= (1.0 - BRAKE) + (BRAKE * V0 / v)
    self.vy *= (1.0 - BRAKE) + (BRAKE * V0 / v)
    self.vx = (1.0 - BRAKE) * self.vx + BRAKE * VX0
    # Bound speed
    self.vx = max(min(self.vx, 4.0), -4.0)
    self.vy = max(min(self.vy, 4.0), -4.0)


def main():
  bk = backends.BackendSDLInit(W, H, BPP, 0, "Gravplots AA")
  assert bk
  print(bk)
  print("Modify source for parameters,")
  print("Kill to terminate ;-)")
  black = bk.context.RGBToPixel(0, 0, 0)

  es = [elem() for i in range(N)]
  while True:
    for e in es:
      for e2 in es:
        if not (e is e2):
          e.grav(e2, 1.0)
    for e in es:
      e.move(1.0)
      if AA:
        x = int((e.x % W) * 0x100)
        y = int(e.y * 0x100)
        if e.vx > 0.2:
          bk.context.gfx.VLineAA(x + 0x100, y - 0x300, y + 0x300, black)
        if e.vx < -0.2:
          bk.context.gfx.VLineAA(x - 0x100, y - 0x300, y + 0x300, black)
        bk.context.gfx.PutPixelAA(x, y, bk.context.RGBToPixel(e.r, e.g, e.b))
      else:
        x = int(e.x % W)
        y = int(e.y)
        if e.vx > 0.2:
          bk.context.gfx.VLine(x + 1, y - 2, y + 2, black)
        if e.vx < -0.2:
          bk.context.gfx.VLine(x - 1, y - 2, y + 2, black)
          bk.context.core.PutPixel(x, y, bk.context.RGBToPixel(e.r, e.g, e.b))
    bk.Poll()
    bk.Flip()
    global TIMEOUT
    if TIMEOUT > 0:
      TIMEOUT -= 1
    if TIMEOUT == 0:
      break
  if SAVETO:
    bk.context.Save(SAVETO)

if __name__ == '__main__':
  main()
