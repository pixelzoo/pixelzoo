SDLCONFIG = sdl-config
PKGCONFIG = pkg-config

SDL_CFLAGS  := $(shell $(SDLCONFIG) --cflags)
SDL_LDFLAGS := $(shell $(SDLCONFIG) --libs) -L/usr/X11R6/lib -lXi

XML_CFLAGS  := $(shell $(PKGCONFIG) --cflags libxml-2.0)
XML_LDFLAGS := $(shell $(PKGCONFIG) --libs libxml-2.0)

CC          := gcc
COPTS       := -g -Wall
CFLAGS      := $(SDL_CFLAGS) $(XML_CFLAGS) -Isrc
ANSI        := -ansi
LIBS        := -lc $(SDL_LDFLAGS) $(XML_LDFLAGS)

TARGETS := test_red_black_tree sdltest sdlgame

OFILES  := $(addprefix obj/,$(addsuffix .o,$(filter-out $(TARGETS),$(basename $(notdir $(wildcard src/*.c))))))
XFILES  := $(addprefix bin/,$(TARGETS))

XMLFILES := t/simple.xml

ifneq (,$(findstring debug,$(MAKECMDGOALS)))
# 'debug' was specified:
DEBUG := -debug
else
DEBUG :=
endif

all: lib targets xml

clean:
	rm -rf obj/* bin/* *~ *.dSYM $(XMLFILES)

test: targets
	bin/sdlgame t/testgame.xml

oldtest: all
	test/testrb.sh && bin/sdltest

targets: $(XFILES)

xml: $(XMLFILES)

lib: $(OFILES)

bin/%:  test/%.c $(OFILES)
	@test -e bin || mkdir bin
	$(CC) $(COPTS) $(CFLAGS) $(LIBS) -o $@ test/$*.c $(OFILES)

.SUFFIXES :

.SECONDARY:

t/simple.xml: perl/zoocompiler.pl perl/Grammar.pm
	perl/zoocompiler.pl >$@

obj/%.o: src/%.c
	@test -e obj || mkdir obj
	$(CC) $(ANSI) $(COPTS) $(CFLAGS) -c $< -o $@
