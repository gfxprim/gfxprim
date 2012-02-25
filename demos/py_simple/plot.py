#!/usr/bin/env python
import gfxprim.core as core
import gfxprim.backends as backends
import gfxprim.gfx as gfx
import sys
from math import *

W = 640
H = 480
EPS = 0.05

env = {}
env['sin'] = sin
env['cos'] = cos
env['sqrt'] = sqrt

def compute(func, x, y_min, y_max):
	env['x'] = x
	try:
		ret = eval(func, env)
	except ZeroDivisionError, d:
		return -1

	return - H * ret / (y_max - y_min) + (H * y_max) / (y_max - y_min)

def x_to_x(x, x_min, x_max):
	return  (x_max - x_min) * x / W + x_min

class plotter(object):
	def __init__(self, func, x_min, x_max, y_min, y_max):
		self.x = 0.0
		self.y = compute(func, x_to_x(self.x, x_min, x_max), y_min, y_max)
		self.y_min = y_min
		self.y_max = y_max
		self.x_min = x_min
		self.x_max = x_max
		self.func = func
	def next(self):
		x = x_to_x(self.x + 1, self.x_min, self.x_max)
		new_y = compute(self.func, x, self.y_min, self.y_max)
		
		inc = 1.0

		while (abs(new_y - self.y) > 1 and inc > EPS):
			x = x_to_x(self.x + inc, self.x_min, self.x_max)
			new_y = compute(self.func, x, self.y_min, self.y_max)
			inc /= 2
		
		self.x += inc
		self.y = new_y

		print self.x, self.y


def main():
	bk = backends.BackendSDLInit(W, H, 0, 0, "Plot AA")
 	assert bk
 	print bk
	print "Modify source for parameters,"
	print "Kill to terminate ;-)"
	print "Usage: function x_min x_max y_min y_max"
 	black = bk.context.RGBToPixel(0, 0, 0)

	x_min = float(sys.argv[2])
	x_max = float(sys.argv[3])
	y_min = float(sys.argv[4])
	y_max = float(sys.argv[5])
	p = plotter(sys.argv[1], x_min, x_max, y_min, y_max) 

	r = 255
	g = 0
	b = 0

 	while p.x < W:
		x = int((p.x + 0.5) * 0x100)
		y = int((p.y + 0.5) * 0x100)
		gfx.PutPixelAA(bk.context, x, y, bk.context.RGBToPixel(int(r), int(g), int(b)))
		bk.Flip()
		p.next()

	while True:
		pass

if __name__ == '__main__':
	main()
