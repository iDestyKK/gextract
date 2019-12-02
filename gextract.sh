#!/bin/bash

if [ $# -ne 2 ]; then
	printf "usage: %s graph.pgm graph.gpak\n" "$0"
	exit 1
fi

rm -rf "__OUTPUT"
mkdir "__OUTPUT"

./graphgen -i 15 100 100 "$1" "__OUTPUT"
./graphpak "__OUTPUT" "$2"

gzip "$2"
mv "$2.gz" "$2"

rm -rf "__OUTPUT"
