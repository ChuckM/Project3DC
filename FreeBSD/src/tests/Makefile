INC= ../../include/p3dc.h
LIB= ../../lib/libp3dc.a
TESTS= test01 test02 test03 test04 test05 test06 test07 \
	test08 test09 test10 test11 test12  test14\
	list001 list002 list003 \
	view1 xfrm00 xfrm01 xfrm02 xfrm03 mem001 \
	bench001 tri
# bench002

CFLAGS+= -g -Wall -Werror -I../../include/ -L../../lib/  
DEBUGFLAGS= -pg -g -DPARANOID_LIST
LDFLAGS = -lp3dc -lpng -lz -lm

all:	$(TESTS)

clean:
	rm -f *.o *.core

clobber: clean
	rm -f $(TESTS) *.png *.log

$(TESTS): $(INC) $(LIB)

../../include/p3dc.h: ../../../src/include/p3dc.h
	@echo "Stripping ^M chars from $(.TARGET)."
	@sed 's///g' < ../../../src/include/p3dc.h > $(.TARGET)

xfrm00 xfrm02 xfrm03: $(.TARGET).c $(LIB) $(INC)
	cc $(CFLAGS) -o $(.TARGET) $(.PREFIX).c $(LDFLAGS)

view1: $(.TARGET).c $(LIB) $(INC)
	cc $(CFLAGS) -o $(.TARGET) $(.PREFIX).c $(LDFLAGS)

test14: $(.TARGET).c $(LIB) $(INC)
	cc $(CFLAGS) -o $(.TARGET) $(.PREFIX).c $(LDFLAGS)

xfrm01: $(.TARGET).c $(LIB) $(INC)
	cc $(CFLAGS) -o $(.TARGET) $(.PREFIX).c $(LDFLAGS)

mem001: $(.TARGET).c $(LIB) $(INC)
	cc $(CFLAGS) -o $(.TARGET) $(.PREFIX).c $(LDFLAGS)

tri: $(.TARGET).c $(LIB) $(INC)
	cc $(CFLAGS) -o $(.TARGET) $(.PREFIX).c $(LDFLAGS)

bench002: $(.TARGET).c $(LIB) $(INC)
	cc $(CFLAGS) -o $(.TARGET) $(.PREFIX).c $(LDFLAGS)

bench001: $(.TARGET).c $(LIB) $(INC)
	cc $(CFLAGS) -o $(.TARGET) $(.PREFIX).c $(LDFLAGS)

list003: $(.TARGET).c $(LIB) $(INC)
	cc $(CFLAGS) -o $(.TARGET) $(.PREFIX).c $(LDFLAGS)

list002: $(.TARGET).c $(LIB) $(INC)
	cc $(CFLAGS) -o $(.TARGET) $(.PREFIX).c $(LDFLAGS)

list001: $(.TARGET).c $(LIB) $(INC)
	cc $(CFLAGS) -o $(.TARGET) $(.PREFIX).c $(LDFLAGS)

test11: $(.TARGET).c $(LIB) $(INC)
	cc $(CFLAGS) -o $(.TARGET) $(.PREFIX).c $(LDFLAGS)

test12: $(.TARGET).c $(LIB) $(INC)
	cc $(CFLAGS) -o $(.TARGET) $(.PREFIX).c $(LDFLAGS)

test10: $(.TARGET).c $(LIB) $(INC)
	cc $(CFLAGS) -o $(.TARGET) $(.PREFIX).c $(LDFLAGS)

test09: $(.TARGET).c $(LIB) $(INC)
	cc $(CFLAGS) -o $(.TARGET) $(.PREFIX).c $(LDFLAGS)

test08: $(.TARGET).c $(LIB) $(INC)
	cc $(CFLAGS) -o $(.TARGET) $(.PREFIX).c $(LDFLAGS)

test07: $(.TARGET).c $(LIB) $(INC)
	cc $(CFLAGS) -o $(.TARGET) $(.PREFIX).c $(LDFLAGS)

test06: $(.TARGET).c $(LIB) $(INC)
	cc $(CFLAGS) -o $(.TARGET) $(.PREFIX).c $(LDFLAGS)

test05: test05.o libp3dc.a $(INC)
	cc $(CFLAGS) -o $(.TARGET) $(.PREFIX).c $(LDFLAGS)

test01:	test01.o libp3dc.a $(INC)
	cc $(CFLAGS) -o $(.TARGET) $(.PREFIX).c $(LDFLAGS)

