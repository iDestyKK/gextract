CC     = g++
CFLAGS = -g
EXTDIR = cn_image

all: main binarise

main: main.cpp $(EXTDIR)/basics.hpp $(EXTDIR)/image_processor.hpp $(EXTDIR)/image_processor.tpp
	$(CC) $(CFLAGS) -o $@ $<

binarise: binarise.cpp $(EXTDIR)/basics.hpp $(EXTDIR)/image_processor.hpp $(EXTDIR)/image_processor.tpp
	$(CC) $(CFLAGS) -o $@ $<

clean:
	$(RM) main binarise
