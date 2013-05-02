#
# Makefile for the Project: 3D Craft library (libp3dc.a)
# $Id:$
#
# This makefile creates local copies of the generic sources
# to make sure that they don't contain DOS type line terminators.
# (Yes, I do a lot of development under VC++ 5.0)
#
# There is however one unique source file that lives here, and
# that is gal_text.c which is a pseudo back end for p3dc that
# basically simulates a frame buffer and uses the PNG library
# to write out snapshots.
#
# The target is the libp3dc.a
#
SRC_DIR = src/lib

DIR = include lib $(SRC_DIR)

SOURCE = ../../../src

SRCS = 	camera.c   clip.c        display.c    draw.c     \
		light.c    list.c        log.c        memory.c   \
		model.c    model_parse.c primitives.c rbtree.c   \
		texture.c  transform.c   util.c       vertex.c   \
		gal_text.c capture.c     writepng.c

OBJS = 	camera.o      clip.o        display.o    draw.o      \
		light.o       list.o        log.o        memory.o    \
		model.o       primitives.o  rbtree.o  				 \
		texture.o     transform.o   util.o       vertex.o    \
		model_parse.o gal_text.o    capture.o    writepng.o

LIB_OBJS := $(OBJS:S!^!lib/!)
SRC_FILES := $(OBJS:S!^!src/lib/!S!.o!.c!)

INC = ../../include/p3dc.h

CFLAGS += -Wall -Werror -I../../include

all: $(INC) libp3dc.a

install: all
	install -c libp3dc.a ../../lib/

clean:
	rm -f *.o

clobber: clean
	rm -f libp3dc.a

libp3dc.a: $(OBJS)
	ar -rv $(.TARGET) $(.ALLSRC)
	ranlib $(.TARGET)

profile: 
	rm *.o
	CFLAGS="-pg" make all
	mv libp3dc.a libp3dc_p.a
	rm *.o

debug:
	rm *.o
	CFLAGS="-g -DPARANOID_LIST" make all
	mv libp3dc.a libp3dc_g.a
	rm *.o

../../include/p3dc.h: $(SOURCE)/include/p3dc.h
	@echo "Stripping ^M chars from $(.TARGET)."
	@sed 's///g' < ../../../src/include/p3dc.h > $(.TARGET)


src/lib:
	mkdir -p src/lib

include:
	mkdir include

lib:
	mkdir lib

obj:
	mkdir obj

changes:
	-@diff --brief -b src/lib/camera.c ../../src/lib/camera.c
	-@diff --brief -b src/lib/clip.c ../../src/lib/clip.c
	-@diff --brief -b src/lib/display.c ../../src/lib/display.c
	-@diff --brief -b src/lib/draw.c ../../src/lib/draw.c
	-@diff --brief -b src/lib/light.c ../../src/lib/light.c
	-@diff --brief -b src/lib/list.c ../../src/lib/list.c
	-@diff --brief -b src/lib/log.c ../../src/lib/log.c
	-@diff --brief -b src/lib/memory.c ../../src/lib/memory.c
	-@diff --brief -b src/lib/model.c ../../src/lib/model.c
	-@diff --brief -b src/lib/model_parse.c ../../src/lib/model_parse.c
	-@diff --brief -b src/lib/primitives.c ../../src/lib/primitives.c
	-@diff --brief -b src/lib/rbtree.c ../../src/lib/rbtree.c
	-@diff --brief -b src/lib/texture.c ../../src/lib/texture.c
	-@diff --brief -b src/lib/transform.c ../../src/lib/transform.c
	-@diff --brief -b src/lib/util.c ../../src/lib/util.c
	-@diff --brief -b src/lib/vertex.c ../../src/lib/vertex.c
	-@diff --brief -b src/lib/capture.c ../../src/lib/capture.c
	-@diff --brief -b include/p3dc.h ../../src/include/p3dc.h

camera.o: $(INC) $(.PREFIX).c

clip.o: $(INC) $(.PREFIX).c

display.o: $(INC) $(.PREFIX).c

draw.o: $(INC) $(.PREFIX).c

light.o: $(INC) $(.PREFIX).c

list.o: $(INC) $(.PREFIX).c

log.o: $(INC) $(.PREFIX).c

memory.o: $(INC) $(.PREFIX).c 

model.o: $(INC) $(.PREFIX).c

primitives.o: $(INC) $(.PREFIX).c

rbtree.o: $(INC) $(.PREFIX).c

texture.o: $(INC) $(.PREFIX).c

transform.o: $(INC) $(.PREFIX).c

util.o: $(INC) $(.PREFIX).c

vertex.o: $(INC) $(.PREFIX).c

model_parse.o: $(INC) $(.PREFIX).c

gal_text.o: $(INC) $(.PREFIX).c

capture.o: $(INC) $(.PREFIX).c

writepng.o: $(INC) $(.PREFIX).c

camera.c: $(SOURCE)/lib/$(.TARGET)
	@echo "Stripping ^M chars from $(.TARGET)."
	@sed 's///g' < $(SOURCE)/lib/$(.TARGET) > $(.TARGET)

clip.c: $(SOURCE)/lib/$(.TARGET)
	@echo "Stripping ^M chars from $(.TARGET)."
	@sed 's///g' < $(SOURCE)/lib/$(.TARGET) > $(.TARGET)

display.c: $(SOURCE)/lib/$(.TARGET)
	@echo "Stripping ^M chars from $(.TARGET)."
	@sed 's///g' < $(SOURCE)/lib/$(.TARGET) > $(.TARGET)

draw.c: $(SOURCE)/lib/$(.TARGET)
	@echo "Stripping ^M chars from $(.TARGET)."
	@sed 's///g' < $(SOURCE)/lib/$(.TARGET) > $(.TARGET)

light.c: $(SOURCE)/lib/$(.TARGET)
	@echo "Stripping ^M chars from $(.TARGET)."
	@sed 's///g' < $(SOURCE)/lib/$(.TARGET) > $(.TARGET)

list.c: $(SOURCE)/lib/$(.TARGET)
	@echo "Stripping ^M chars from $(.TARGET)."
	@sed 's///g' < $(SOURCE)/lib/$(.TARGET) > $(.TARGET)

log.c: $(SOURCE)/lib/$(.TARGET)
	@echo "Stripping ^M chars from $(.TARGET)."
	@sed 's///g' < $(SOURCE)/lib/$(.TARGET) > $(.TARGET)

memory.c: $(SOURCE)/lib/$(.TARGET)
	@echo "Stripping ^M chars from $(.TARGET)."
	@sed 's///g' < $(SOURCE)/lib/$(.TARGET) > $(.TARGET)

model.c: $(SOURCE)/lib/$(.TARGET)
	@echo "Stripping ^M chars from $(.TARGET)."
	@sed 's///g' < $(SOURCE)/lib/$(.TARGET) > $(.TARGET)

model_parse.c: $(SOURCE)/lib/$(.TARGET)
	@echo "Stripping ^M chars from $(.TARGET)."
	@sed 's///g' < $(SOURCE)/lib/$(.TARGET) > $(.TARGET)

primitives.c: $(SOURCE)/lib/$(.TARGET)
	@echo "Stripping ^M chars from $(.TARGET)."
	@sed 's///g' < $(SOURCE)/lib/$(.TARGET) > $(.TARGET)

rbtree.c: $(SOURCE)/lib/$(.TARGET)
	@echo "Stripping ^M chars from $(.TARGET)."
	@sed 's///g' < $(SOURCE)/lib/$(.TARGET) > $(.TARGET)

texture.c: $(SOURCE)/lib/$(.TARGET)
	@echo "Stripping ^M chars from $(.TARGET)."
	@sed 's///g' < $(SOURCE)/lib/$(.TARGET) > $(.TARGET)

transform.c: $(SOURCE)/lib/$(.TARGET)
	@echo "Stripping ^M chars from $(.TARGET)."
	@sed 's///g' < $(SOURCE)/lib/$(.TARGET) > $(.TARGET)

util.c: $(SOURCE)/lib/$(.TARGET)
	@echo "Stripping ^M chars from $(.TARGET)."
	@sed 's///g' < $(SOURCE)/lib/$(.TARGET) > $(.TARGET)

vertex.c: $(SOURCE)/lib/$(.TARGET)
	@echo "Stripping ^M chars from $(.TARGET)."
	@sed 's///g' < $(SOURCE)/lib/$(.TARGET) > $(.TARGET)

writepng.c: $(SOURCE)/lib/$(.TARGET)
	@echo "Stripping ^M chars from $(.TARGET)."
	@sed 's///g' < $(SOURCE)/lib/$(.TARGET) > $(.TARGET)

capture.c: $(SOURCE)/lib/$(.TARGET)
	@echo "Stripping ^M chars from $(.TARGET)."
	@sed 's///g' < $(SOURCE)/lib/$(.TARGET) > $(.TARGET)
