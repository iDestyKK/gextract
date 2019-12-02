CC     = g++
CFLAGS = -O3 --std=c++11
EXTDIR = cn_image

all: graphgen graphpak

graphgen: graphgen.cpp
	$(CC) $(CFLAGS) -o $@ $<

graphpak: graphpak.cpp
	$(CC) $(CFLAGS) -o $@ $<

clean:
	$(RM) graphgen graphpak

package:
	tar -cvf gextract.tar.xz gextract.sh makefile graphgen.cpp graphpak.cpp cn_image samples web
