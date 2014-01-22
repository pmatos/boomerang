######################################################
# File: Makefile.in
# Desc: Makefile for boomerang
#       Makes and tests all object files all directories
#
######################################################

###############
# Introduction
#
# This Makefile controls the whole build process of the Boomerang decompiler, except for the loaders.
# The loaders have their own makefile (loaders/Makefile).
# The gui's also have to take care of themselves.
#
# Index of this makefile: 
#
# - Variables detected by ./configure
# - Detection of OS specific settings
# - Compiler settings
# - Main rules
# - Loaders
# - Detection of machine types and loaders
# - Lots of object files
# - Conditional rules
# - Rules for compiling
# - Unit testing (broken?)
# - Cleaning up
# - Dependencies
#

ACLOCAL_AMFLAGS = -I .

DISTCHECK_CONFIGURE_FLAGS = \
	$(NULL)

STD_DIRS = \
	loader \
	db \
	transform \
	util \
	frontend \
	. \
	$(NULL)

SUBDIRS = $(STD_DIRS)
DIST_SUBDIRS = $(STD_DIRS)

nobase_include_HEADERS = include/*.h

bin_PROGRAMS = boomerang bffDump
noinst_LIBRARIES = libdriver.a
bffDump_sources = loader/bffDump.cpp
boomerang_SOURCES = boomerang.cpp $(STATIC_OBJS) $(GENSSL)
boomerang_LDADD = libdriver.a
libdriver_a_SOURCES= driver.cpp include/boomerang.h

