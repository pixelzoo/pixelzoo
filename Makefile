PERL        := /opt/local/bin/perl
#PERL        := /usr/bin/perl
XMLLINT     := /usr/bin/xmllint

SDLCONFIG = sdl-config
PKGCONFIG = pkg-config

SDL_CFLAGS  := $(shell $(SDLCONFIG) --cflags)
SDL_LDFLAGS := $(shell $(SDLCONFIG) --static-libs) -L/usr/X11R6/lib -lXi

CC          := gcc
AR          := ar
COPTS       := -g -Wall
CFLAGS      := $(SDL_CFLAGS) -Isrc
ANSI        := -ansi -std=c99
LIBS        := -lc $(SDL_LDFLAGS)
ARFLAGS     := -rcvs

TARGETS   := test_red_black_tree sdlgame pztest hardcoded-eloise-pztest
LIBDIR    := lib
LIBNAME   := pixelzoo
LIBTARGET := $(LIBDIR)/lib$(LIBNAME).a

OFILES  := $(addprefix obj/,$(addsuffix .o,$(filter-out $(TARGETS),$(basename $(notdir $(wildcard src/*.c))))))
XFILES  := $(addprefix bin/,$(TARGETS))

XMLTESTFILES := t/simple.copy.xml t/compiled.copy.xml

all: libtargets targets

clean:
	rm -rf obj/* bin/* lib/* *~ *.dSYM $(XMLTESTFILES)

cleanxml:
	rm $(XMLTESTFILES)

sdl: targets
	bin/sdlgame -g t/testgame.xml -l t/movelog.xml -b t/board.xml -r t/revcomp.xml

timed-sdl: targets
	bin/sdlgame -g t/testgame.xml -l t/movelog.xml -b t/board.xml -r t/revcomp.xml -t 5000000000

# Targets

targets: $(XFILES)

libtargets: $(LIBTARGET)

bin/%:  tsrc/%.c $(LIBTARGET)
	@test -e bin || mkdir bin
	$(CC) $(COPTS) $(CFLAGS) $(LIBS) -L$(LIBDIR) -l$(LIBNAME) -o $@ tsrc/$*.c

obj/%.o: src/%.c
	@test -e obj || mkdir obj
	$(CC) $(ANSI) $(COPTS) $(CFLAGS) -c $< -o $@

$(LIBTARGET): $(OFILES)
	@test -e lib || mkdir lib
	$(AR) $(ARFLAGS) $(LIBTARGET) $(OFILES)

.SUFFIXES :

.SECONDARY:


# Test collections

test: replay-test

all-tests: replay-test sdl-test compiler-test

replay-test: eloise-pztest hardcoded-eloise-pztest

sdl-test: eloise-sdl-test

compiler-test: xml-valid-test xml-build-test


# Simple replay test, constructed as follows:
#
#  % bin/sdlgame -g t/testgame.xml -l t/movelog.xml -b t/board.xml -t 5000000000
#
#   ...trace the letters "eloise" on the screen for the 20 seconds or so that the test runs
#
#  % cp t/board.xml t/eloise_board.xml
#  % cp t/movelog.xml t/eloise_movelog.xml
#  % cp t/testgame.xml t/eloise_queue.xml
#
#   ...then paste t/eloise_movelog.xml into t/eloise_queue.xml
#
# eloise_X.xml should be identical to eloise_copy_X.xml for X in { movelog, board }
ELOISE_TEST_ARGS = -g t/eloise_queue.xml -l t/eloise_copy_movelog.xml -b t/eloise_copy_board.xml -t 5000000000
eloise-sdl-test: targets
	bin/sdlgame -d $(ELOISE_TEST_ARGS)
	diff t/eloise_movelog.xml t/eloise_copy_movelog.xml
	diff t/eloise_board.xml t/eloise_copy_board.xml

eloise-pztest: targets
	bin/pztest $(ELOISE_TEST_ARGS)
	diff t/eloise_movelog.xml t/eloise_copy_movelog.xml
	diff t/eloise_board.xml t/eloise_copy_board.xml

hardcoded-eloise-pztest: targets
	time bin/hardcoded-eloise-pztest

eloise: eloise-sdl-test eloise-pztest

# XML validation tests
# These should produce no output
xml-valid-test:
	$(XMLLINT) --dtdvalid Zoo/dtd/game.dtd --noout t/testgame.xml
	$(XMLLINT) --dtdvalid Zoo/dtd/proto.dtd --noout t/proto.xml

# XML generation tests

xml-build-test: $(XMLTESTFILES)

# Build a simple world using the Perl API.
t/simple.copy.xml: perl/simplezoo.pl Zoo/lib/Grammar.pm Zoo/lib/Level.pm
	$(PERL) perl/simplezoo.pl -xmllint $(XMLLINT) -proto t/proto.copy.xml -out $@ -verbose
	diff t/proto.xml t/proto.copy.xml
	diff t/simple.xml t/simple.copy.xml

xml-debug: perl/simplezoo.pl Zoo/lib/Grammar.pm Zoo/lib/Level.pm
	$(PERL) perl/simplezoo.pl -xmllint $(XMLLINT) -proto t/proto.copy.xml -out t/simple.copy.xml -debug

t/compiled.copy.xml: t/proto.xml
	$(PERL) perl/zoocompiler.pl $< >$@
	diff t/compiled.xml t/compiled.copy.xml


# Polymers
t/poly.xml: perl/polyzoo.pl
	perl/polyzoo.pl >$@

polysdl: t/poly.xml
	bin/sdlgame -g $<

# Documentation
doc: dtddoc-doc doxygen-doc

doxygen-doc:
	cd src; doxygen

dtddoc-doc:
	cd Zoo/dtd; make

README.html: README.md
	perl -e 'use Text::Markdown "markdown";print markdown(join("",<>))' $< >$@

# emscripten
bin/pztest.js:
	/usr/local/src/emscripten/emmake make -f Makefile.em
