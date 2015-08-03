CPPFLAGS += 
LDFLAGS += -llua 

SRC = $(filter-out testbox.cc, $(wildcard *.cc))
LIB = luax.so
CLEAN = preload.h preload.luac

luainterface.o: luainterface.cc preload.h

preload.h: preload.luac
	xxd -i $< > $@

preload.luac: $(wildcard preload/*.lua)
	luac -o $@ $^ 

include ../util/config.mk
