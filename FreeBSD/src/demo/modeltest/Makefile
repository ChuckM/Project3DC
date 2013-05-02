#
# Build model test
#
INC = ../../../include/p3dc.h
LIB = ../../../lib/libp3dc.a
CFLAGS= -g -I../../../include/ -L../../../lib/

all: modeltest

pix: modeltest.png
	install -c modeltest.png ../../../pix

modeltest.png: modeltest
	modeltest

modeltest: modeltest.o $(INC) $(LIB)
	cc $(CFLAGS) -o modeltest modeltest.c -lp3dc -lpng -lz -lm
