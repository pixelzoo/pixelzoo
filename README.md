PixelZoo
========

A client-server architecture for 2D cellular automata simulations.
(See doc/index.html file for developer documentation.)


Introduction
============

PixelZoo is a project-in-progress to build multiplayer
turn-based games using fast two-dimensional stochastic Cellular
Automata (CA).

The graphics aesthetic is experimental and minimal. Particles in the
game are just colored pixels. There is some textual feedback: particle
names, "speech balloons", an output console. The design goal is that
particles are characterized by the player largely according to their
behavior as microscopic agents.

The CA's state machines are programmed using a minimal rule language
with only a few elements: integer variables, addition, a C-like
"switch", UNIX-like signals ("messages"). This makes it very easy to
analyze the programs (i.e. predict their CPU usage), while being rich
enough to implement models from physics, chemistry and biology, e.g.

- simple models of ideal gases
- Brownian motion and Brownian superprocesses
- diffusion-limited-aggregation and crystal growth
- chemical kinetics, the Belousov–Zhabotinsky reaction
- the two-dimensional Ising model of magnetic spin arrays
- mesoscale dynamics of polymers and entangled polymer melts
- the Susceptible-Infected-Recovered (SIR) model of epidemiology
- neutral drift models in population genetics and ecology
- forest fires, ants, bacteria, traffic, crowds, etc.

In the single-player (tutorial) version of the game, the player
attempts to solve various puzzles and challenges (e.g. "maintain an
ecology of >N different species for time >T, in the face of various
natural disasters").

In the multi-player version, which is turn-based, players compete for
control of self-contained playing boards ("worlds"). Players can also
create their own (self-replicating) CA programs, and trade these
in-game.


State of the project
====================

We said it was a work-in-progress, didn't we?

Mostly completed parts include:

- an XML schema for describing state-machine rules and level goals
- a core ANSI C library that runs games and verifies moves
- Perl modules for generating and compiling levels
- tests and (some) documentation

Work in progress includes:

- a RESTful multiplayer game server, based on Perl Catalyst
- clients for iOS, Android and Simple DirectMedia Layer (SDL)


Dependencies
============

Depending on what you want to build, you will need some other stuff
installed.

Core client engine
------------------

C code in src/, compiles into lib/


SDL client
----------

C code in tsrc/sdltest.c

Requires Simple DirectMedia Layer (SDL).

- http://www.libsdl.org/


iOS client
----------

Objective-C code and Xcode IDE files in xcode/pixelzoo/

Requires Xcode IDE and iOS development framework.

- http://developer.apple.com/xcode/


Android client
--------------

Java/C code and Eclipse IDE files in android/

Requires Eclipse IDE and Android SDK/NDK.

- http://www.eclipse.org/
- http://developer.android.com/sdk/
- http://developer.android.com/sdk/ndk/


Server
------

Perl code in perl/ and Zoo/

The game XML compiler, perl/zoocompiler.pl, requires various modules from CPAN,
notably XML::Twig and Moose.

- http://www.cpan.org/

The server framework in Zoo/ requires Perl Catalyst, and a few other modules.
Do 'cd Zoo; perl Makefile.PL; make' to install other dependencies from CPAN.

- http://www.catalystframework.org/
