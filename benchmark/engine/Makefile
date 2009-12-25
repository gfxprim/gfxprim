OBJECTS=TestHtml.o TestUtils.o
LIB=TestUtils.a

$(LIB): $(OBJECTS)

all: $(LIB)

$(LIB):
	ar rcs $(LIB) $(OBJECTS)

clean:
	rm -f $(OBJECTS) $(LIB)
