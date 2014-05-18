SUBDIRS = \
	loader \
	util \
	type \
	transform \
	codegen \
	c \
	db \
	frontend \
	.

AM_CPPFLAGS = \
	$(GC_CFLAGS)

noinst_LTLIBRARIES = \
	libboomerang.la

bin_PROGRAMS = \
	boomerang

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

boomerang_SOURCES = \
	driver.cpp
boomerang_LDADD = \
	libboomerang.la \
	$(GC_LIBS)
