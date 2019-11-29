CC     = g++
CFLAGS = -g
EXTDIR = cn_image

all: main

main: main.cpp $(EXTDIR)/basics.hpp $(EXTDIR)/image_processor.hpp $(EXTDIR)/image_processor.tpp
	$(CC) $(CFLAGS) -o $@ $<

clean:
	$(RM) main
