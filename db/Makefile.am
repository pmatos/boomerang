AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/c \
	$(GC_CFLAGS)


noinst_LTLIBRARIES = \
	libxmlprogparser.la \
	libdb.la

libxmlprogparser_la_SOURCES = \
	xmlprogparser.cpp
libxmlprogparser_la_LIBADD = \
	$(EXPAT_LIBS)
libxmlprogparser_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	$(EXPAT_CFLAGS)

libdb_la_SOURCES = \
	basicblock.cpp \
	cfg.cpp \
	dataflow.cpp \
	exp.cpp \
	insnameelem.cpp \
	insnameelem.h \
	managed.cpp \
	operstrings.h \
	proc.cpp \
	prog.cpp \
	register.cpp \
	rtl.cpp \
	signature.cpp \
	sslinst.cpp \
	sslparser.y \
	sslscanner.l \
	statement.cpp \
	table.cpp \
	table.h \
	visitor.cpp
libdb_la_LIBADD = \
	$(GC_LIBS)


check_LTLIBRARIES = \
	libCfgTest.la \
	libDfaTest.la \
	libExpTest.la \
	libParserTest.la \
	libProcTest.la \
	libProgTest.la \
	libRtlTest.la \
	libStatementTest.la


libCfgTest_la_SOURCES = \
	CfgTest.cpp \
	CfgTest.h
libCfgTest_la_LIBADD = \
	$(CPPUNIT_LIBS)
libCfgTest_la_CPPFLAGS = \
	-I$(top_srcdir)/frontend \
	$(AM_CPPFLAGS) \
	$(CPPUNIT_CFLAGS)

libDfaTest_la_SOURCES = \
	DfaTest.cpp \
	DfaTest.h
libDfaTest_la_LIBADD = \
	$(CPPUNIT_LIBS)
libDfaTest_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	$(CPPUNIT_CFLAGS)

libExpTest_la_SOURCES = \
	ExpTest.cpp \
	ExpTest.h
libExpTest_la_LIBADD = \
	$(CPPUNIT_LIBS)
libExpTest_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	$(CPPUNIT_CFLAGS)

libParserTest_la_SOURCES = \
	ParserTest.cpp \
	ParserTest.h
libParserTest_la_LIBADD = \
	$(CPPUNIT_LIBS)
libParserTest_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	$(CPPUNIT_CFLAGS)

libProcTest_la_SOURCES = \
	ProcTest.cpp \
	ProcTest.h
libProcTest_la_LIBADD = \
	$(CPPUNIT_LIBS)
libProcTest_la_CPPFLAGS = \
	-I$(top_srcdir)/frontend \
	$(AM_CPPFLAGS) \
	$(CPPUNIT_CFLAGS)

libProgTest_la_SOURCES = \
	ProgTest.cpp \
	ProgTest.h
libProgTest_la_LIBADD = \
	$(CPPUNIT_LIBS)
libProgTest_la_CPPFLAGS = \
	-I$(top_srcdir)/frontend \
	$(AM_CPPFLAGS) \
	$(CPPUNIT_CFLAGS)

libRtlTest_la_SOURCES = \
	RtlTest.cpp \
	RtlTest.h
libRtlTest_la_LIBADD = \
	$(CPPUNIT_LIBS)
libRtlTest_la_CPPFLAGS = \
	-I$(top_srcdir)/frontend \
	$(AM_CPPFLAGS) \
	$(CPPUNIT_CFLAGS)

libStatementTest_la_SOURCES = \
	StatementTest.cpp \
	StatementTest.h
libStatementTest_la_LIBADD = \
	$(CPPUNIT_LIBS)
libStatementTest_la_CPPFLAGS = \
	-I$(top_srcdir)/frontend \
	$(AM_CPPFLAGS) \
	$(CPPUNIT_CFLAGS)


TESTS = $(check_PROGRAMS)
check_PROGRAMS = \
	checkstrings

checkstrings_SOURCES = \
	checkstrings.cpp


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
	sslparser.h \
	sslscanner.h

CLEANFILES = *.output
