%
% Compile to svg by:
%
% mptopdf coordinates.mp; inkscape -l coordinates-1.svg coordinates-1.pdf
%
beginfig(1)
	% Draw grid
	u := 48;
	w := 9;
	h := 7;

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

	% Draw labels
	dotlabel.top(btex (0, 0) etex, (u/2, h * u - u/2));
	dotlabel.top(btex (w-1, h-1) etex, (w * u - u/2, u/2));
	
	dotlabel.urt(btex (1.5, 2) etex, (2 * u, h * u - 2 * u - u/2));

endfig;
end
