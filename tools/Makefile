# include ../util.mk

HOST_ENV := $(shell uname 2>/dev/null || echo Unknown)
HOST_ENV := $(patsubst MINGW%,MinGW,$(HOST_ENV))

CC           := gcc
CXX          := g++
CFLAGS       := -I . -Wall -Wextra -Wno-unused-parameter -pedantic -O2 -s
LDFLAGS      := -lm
ALL_PROGRAMS := n64graphics

default: all

n64graphics_SOURCES := n64graphics.c utils.c
n64graphics_CFLAGS  := -DN64GRAPHICS_STANDALONE


all-except-recomp: $(LIBAUDIOFILE) 

all: $(ALL_PROGRAMS)

clean:
	$(RM) $(ALL_PROGRAMS)

define COMPILE
$(1): $($1_SOURCES)
	$$(CC) $(CFLAGS) $($1_CFLAGS) $$^ -o $$@ $($1_LDFLAGS) $(LDFLAGS)
endef


$(foreach p,$(ALL_PROGRAMS),$(eval $(call COMPILE,$(p))))

.PHONY: all clean default
