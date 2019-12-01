CC     = g++
CFLAGS = -O3
EXTDIR = cn_image

all: graphgen graphpak

graphgen: graphgen.cpp
	$(CC) $(CFLAGS) -o $@ $<

graphpak: graphpak.cpp
	$(CC) $(CFLAGS) -o $@ $<

clean:
	$(RM) graphgen graphpak
