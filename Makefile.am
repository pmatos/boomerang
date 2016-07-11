SUBDIRS = \
	loader \
	util \
	type \
	transform \
	transformations \
	codegen \
	c \
	db \
	frontend \
	signatures \
	. \
	qtgui2

AM_CPPFLAGS = \
	-DDATADIR='"$(pkgdatadir)"' \
	$(GC_CFLAGS)


pkglib_LTLIBRARIES = \
	libboomerang.la

libboomerang_la_SOURCES = \
	boomerang.cpp \
	log.cpp
libboomerang_la_LIBADD = \
	$(top_builddir)/db/libdb.la \
	$(top_builddir)/db/libxmlprogparser.la \
	$(top_builddir)/c/libansi-c.la \
	$(top_builddir)/codegen/libcodegen.la \
	$(top_builddir)/util/libutil.la \
	$(top_builddir)/frontend/libfrontend.la \
	$(top_builddir)/type/libtype.la \
	$(top_builddir)/loader/libBinaryFileFactory.la \
	$(top_builddir)/transform/libtransform.la \
	$(GC_LIBS)
libboomerang_la_LDFLAGS = -no-undefined -avoid-version


bin_PROGRAMS = \
	boomerang

boomerang_SOURCES = \
	driver.cpp
boomerang_LDADD = \
	libboomerang.la \
	$(GC_LIBS)


TESTS = $(check_PROGRAMS)
check_PROGRAMS = \
	testAll

testAll_SOURCES = \
	testAll.cpp
testAll_LDADD = \
	$(top_builddir)/db/libExpTest.la \
	$(top_builddir)/db/libProgTest.la \
	$(top_builddir)/db/libProcTest.la \
	$(top_builddir)/db/libRtlTest.la \
	$(top_builddir)/db/libParserTest.la \
	$(top_builddir)/type/libTypeTest.la \
	$(top_builddir)/frontend/libFrontSparcTest.la \
	$(top_builddir)/frontend/libFrontPentTest.la \
	$(top_builddir)/c/libCTest.la \
	$(top_builddir)/db/libStatementTest.la \
	$(top_builddir)/db/libCfgTest.la \
	$(top_builddir)/db/libDfaTest.la \
	$(top_builddir)/loader/libBinaryFileStub.la \
	libboomerang.la \
	$(CPPUNIT_LIBS)
testAll_CPPFLAGS = \
	-I$(top_srcdir)/db \
	-I$(top_srcdir)/type \
	-I$(top_srcdir)/frontend \
	-I$(top_srcdir)/c \
	-I$(top_srcdir)/util
