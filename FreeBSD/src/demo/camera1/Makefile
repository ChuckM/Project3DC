#
# Build model test
#
INC = ../../../include/p3dc.h
LIB = ../../../lib/libp3dc_g.a
CFLAGS= -g -I../../../include/ -L../../../lib/

all: camera1

pix: camera.png
	install -c camera.png ../../../pix

camera.png: camera1
	camera1

camera1: camera1.o basic_scene.o $(INC) $(LIB)
	cc $(CFLAGS) -o camera1 camera1.o basic_scene.o -lp3dc_g -lpng -lz -lm
