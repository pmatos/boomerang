######################################################
# File: Makefile
# Desc: Makefile for the transformation (transform) directory
#
######################################################

pkglib_LTLIBRARIES = libtransform.la

libtransform_la_SOURCES = \
		transformer.cpp \
		rdi.cpp \
		generic.cpp \
		transformation-parser.cpp \
		transformation-scanner.cpp \
		$(NULL)
			
noinst_HEADERS = \
		rdi.h \
		generic.h \
		transformation-parser.h \
		transformation-scanner.h \
		$(NULL)

libtransform_la_LDFLAGS = -release $(VERSION)
