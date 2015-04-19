AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/db

noinst_LTLIBRARIES = \
	libansi-c.la

check_PROGRAMS = \
	testC

libansi_c_la_SOURCES = \
	ansi-c-parser.y \
	ansi-c-scanner.l

# Do not remove, this has a helpful side-effect of convincing
# make to compile the ansi-c files in C++ mode.
testC_SOURCES = \
	testC.cc \
	CTest.cpp \
	CTest.h
# FIXME:  Shouldn't need to link all of boomerang to run this test.
testC_LDADD = \
	$(top_builddir)/libboomerang.la \
	$(CPPUNIT_LIBS)
testC_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	$(CPPUNIT_CFLAGS)

check-local: $(check_PROGRAMS)
	./testC

# Various, fragile hacks to use flex++/bison++
# Tested with GNU automake 1.11.5, GNU make 3.82
#
# 1.  Set YACC and LEX to keep automake happy.  Otherwise it refuses to
#     finish, complaining that you should add AC_PROG_YACC and AC_PROG_LEX to
#     configure.ac.  Neither of those macros are useful to us.
#
# 2.  Define our own rules for converting .y/.l to .cpp/.h.  Avoid ylwrap,
#     which is unnecessary since the output files don't need to be renamed.
#
# 3.  Convince make to generate .cpp/.h using our rules instead of .c/.h using
#     its built-in rules or the rules supplied by automake.  The order
#     of SUFFIXES is somehow important.  Move the -d flag from AM_YFLAGS
#     to the rule to prevent automake from generating a .h -> .c
#     dependency when it scans AM_YFLAGS for that flag.
#
# 4.  To bootstrap, add the .h files to BUILT_SOURCES.
#
YACC = $(BISONPP)
AM_YFLAGS = -v --debug
LEX = $(FLEXPP)
AM_LFLAGS =

SUFFIXES = .cpp .h .y .l
%.cpp %.h: %.y
	$(YACCCOMPILE) -o $*.cpp -d $<
%.cpp %.h: %.l
	$(LEXCOMPILE) -o$*.cpp -h $<

BUILT_SOURCES = \
	ansi-c-parser.h \
	ansi-c-scanner.h

CLEANFILES = *.output
