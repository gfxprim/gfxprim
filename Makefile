LIBRARY=gfxprim.a

gfxprim.a: circle.o clear.o ellipse.o getpixel.o line.o rect.o setpixel.o triangle.o
	ar crus $@ $^

clean:
	rm -f *.o
	rm -f $(LIBRARY)
	cd tests && $(MAKE) clean

tar: clean
	cd .. && tar cjf gfxprim-`date +%Y-%b-%d`.tar.bz2 gfxprim
