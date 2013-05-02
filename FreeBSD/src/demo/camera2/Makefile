#
# Build model test
#
INC = ../../../include/p3dc.h
LIB = ../../../lib/libp3dc_g.a
CFLAGS= -g -I../../../include/ -L../../../lib/

all: camera2

pix: camera2.png
	install -c camera2.png ../../../pix

camera2.png: camera2
	camera2

camera2: camera2.o basic_scene.o $(INC) $(LIB)
	cc $(CFLAGS) -o camera2 camera2.o basic_scene.o -lp3dc_g -lpng -lz -lm
