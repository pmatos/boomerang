AM_CPPFLAGS = \
	-I$(top_srcdir)/include


frontenddir = \
	$(pkgdatadir)/frontend

nobase_frontend_DATA = \
	machine/mips/mips.ssl \
	machine/pentium/pentium.ssl \
	machine/ppc/ppc.ssl \
	machine/st20/st20.ssl \
	machine/sparc/sparc.ssl


noinst_LTLIBRARIES = \
	libfrontend.la \
	libfrontsparc.la \
	libfrontpentium.la \
	libfrontst20.la \
	libfrontppc.la \
	libfrontmips.la

libfrontend_la_SOURCES = \
	frontend.cpp \
	njmcDecoder.cpp
libfrontend_la_LIBADD = \
	libfrontsparc.la \
	libfrontpentium.la \
	libfrontst20.la \
	libfrontppc.la \
	libfrontmips.la
libfrontend_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/c

libfrontsparc_la_SOURCES = \
	sparcdecoder.cpp \
	sparcdecoder.h \
	sparcfrontend.cpp \
	sparcfrontend.h

libfrontpentium_la_SOURCES = \
	pentiumdecoder.cpp \
	pentiumdecoder.h \
	pentiumfrontend.cpp \
	pentiumfrontend.h

libfrontst20_la_SOURCES = \
	st20decoder.cpp \
	st20decoder.h \
	st20frontend.cpp \
	st20frontend.h

libfrontppc_la_SOURCES = \
	ppcdecoder.cpp \
	ppcdecoder.h \
	ppcfrontend.cpp \
	ppcfrontend.h

libfrontmips_la_SOURCES = \
	mipsdecoder.cpp \
	mipsdecoder.h \
	mipsfrontend.cpp \
	mipsfrontend.h


check_LTLIBRARIES = \
	libFrontPentTest.la \
	libFrontSparcTest.la

libFrontPentTest_la_SOURCES = \
	FrontPentTest.cpp \
	FrontPentTest.h
libFrontPentTest_la_LIBADD = \
	$(CPPUNIT_LIBS)
libFrontPentTest_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	$(CPPUNIT_CFLAGS)

libFrontSparcTest_la_SOURCES = \
	FrontSparcTest.cpp \
	FrontSparcTest.h
libFrontSparcTest_la_LIBADD = \
	$(CPPUNIT_LIBS)
libFrontSparcTest_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	$(CPPUNIT_CFLAGS)
