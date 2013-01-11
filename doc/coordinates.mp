def draw_grid = 
	% Draw grid
	for i=0 upto h:
		draw (0, i * u) -- (w * u, i * u);
	endfor

	for i=0 upto w:
		draw (i * u, 0) -- (i * u, h * u);
	endfor

	% Draw frame
	pair A, B, C, D;
	
	A = (0, 0);
	B = (w * u, 0);
	C = (w * u, h * u);
	D = (0, h * u);

	drawarrow D -- A withpen pencircle scaled 4;
	draw A -- B withpen pencircle scaled 2;
	draw B -- C withpen pencircle scaled 2;
	drawarrow D -- C withpen pencircle scaled 4;
enddef;

vardef pixel_coord(expr x, y) = 
	px := u/2 + x * u;
	py := h * u - u/2 - u * y;
	(px,py)
enddef;

def fill_pixel(expr x, y, col) =
	fill unitsquare scaled u shifted pixel_coord(x - 0.5, y + 0.5) withcolor col
enddef;

beginfig(1)
	u = 48;
	w = 11;
	h = 6;
	
	% Draw labels
	dotlabel.top(btex (0, 0) etex, pixel_coord(0, 0));
	dotlabel.top(btex (w-1, h-1) etex, pixel_coord(w-1, h-1));
	dotlabel.urt(btex (1.5, 2) etex, pixel_coord(1.5, 2));
	
	% Draw line
	lxs = 5;
	lxe = 9;
	ly = 3;

	for i = lxs upto lxe:
		fill_pixel(i, ly, (0.6, 0.6, 1));
	endfor

	dotlabel.top(btex (5, 3) etex, pixel_coord(lxs, ly));
	dotlabel.top(btex (9, 3) etex, pixel_coord(lxe, ly));
	
	draw_grid;
endfig;

beginfig(2)
	u := 48;
	w := 11;
	h := 6;
	
	% Draw labels
	dotlabel.top(btex (0, 0) etex, pixel_coord(0, 0));
	
	for i = 2 upto 9:
		for j = 1 upto 4:
			fill_pixel(i, j, (0.6, 1, 0.6));
		endfor
	endfor

	% Draw rectangle
	dotlabel.top(btex (2, 1) etex, pixel_coord(2, 1));
	dotlabel.top(btex (9, 4) etex, pixel_coord(9, 4));

	draw_grid;
endfig;

end
