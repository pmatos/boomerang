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

noinst_LTLIBRARIES = \
	libdriver.la

bin_PROGRAMS = \
	boomerang

libdriver_la_SOURCES = \
	driver.cpp
libdriver_la_LIBADD = \
	$(GC_LIBS)
libdriver_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	$(GC_CFLAGS)

boomerang_SOURCES = \
	boomerang.cpp \
	log.cpp
boomerang_LDADD = \
	libdriver.la \
	$(top_builddir)/db/libdb.la \
	$(top_builddir)/c/libansi-c.la \
	$(top_builddir)/codegen/libcodegen.la \
	$(top_builddir)/util/libutil.la \
	$(top_builddir)/frontend/libfrontend.la \
	$(top_builddir)/type/libtype.la \
	$(top_builddir)/loader/libBinaryFileFactory.la \
	$(top_builddir)/transform/libtransform.la
