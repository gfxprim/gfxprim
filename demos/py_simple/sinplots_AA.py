#!/usr/bin/env python
import gfxprim.core as core
import gfxprim.backends as backends
import gfxprim.gfx as gfx
import random
from math import sin

AA = True
W = 320
H = 240
N = 10

class plotter(object):
  def __init__(self):
    self.x0 = random.uniform(0, W)
    self.y0 = random.uniform(H*0.2, H*0.8)
    self.a = H * (0.06 + random.uniform(0, 0.1) + random.uniform(0, 0.2))
    self.period = 4.0 + random.expovariate(4.0/W)
    self.speed = random.uniform(0.04, min(max(0.05, 0.5 * self.period / self.a), 1.0))
  def pos(self, t):
    x = self.x0 + self.speed * t
    y = self.y0 + sin(x / self.period) * self.a
    return (x % W, y)
  def color(self, t):
    return (
      128 + 120 * sin(t / (self.a + 10.0)),
      128 + 120 * sin(t / (self.y0 + 3.0)),
      128 + 120 * sin(t / (self.x0 + 5.0)))


def main():
  bk = backends.BackendSDLInit(W, H, 16, 0, "Sinplots AA")
  assert bk
  print(bk)
  print("Modify source for parameters,")
  print("Kill to terminate ;-)")
  black = bk.pixmap.RGBToPixel(0, 0, 0)

  ps = [plotter() for i in range(N)]
  t = random.uniform(0.0, 10.0 * W)
  while True:
    t += 1.0
    for p in ps:
      (x, y) = p.pos(t)
      (r, g, b) = p.color(t)
      if AA:
        x = int(x * 0x100)
        y = int(y * 0x100)
        bk.pixmap.gfx.VLineAA(x + 0x100, y - 0x200, y + 0x200, black)
        bk.pixmap.gfx.PutPixelAA(x, y, bk.pixmap.RGBToPixel(int(r), int(g), int(b)))
      else:
        x = int(x)
        y = int(y)
        bk.pixmap.gfx.VLine(x + 1, y - 2, y + 2, black)
        bk.pixmap.core.PutPixel(x, y, bk.pixmap.RGBToPixel(int(r), int(g), int(b)))
    bk.Flip()

if __name__ == '__main__':
  main()
