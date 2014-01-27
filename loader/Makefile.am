######################################################
# File: Makefile
#
# Desc: Makefile for the Loader library files.
#   This loader is lightly adapted from the loader in UQBT
#   Purpose is to load the input binary file into memory, with calls to iterate through sections,
#	find entry points, symbols, etc.
#   The main library is BinaryFile, which calls eactly one of ElfBinaryFile etc as needed.
#
# Prerequisite libraries: none
#
######################################################

AM_CPPFLAGS = -I$(top_srcdir)/include

noinst_LTLIBRARIES = \
	libmicroX86dis.la \
	libBinaryFile.la \
	libBinaryFileFactory.la

libmicroX86dis_la_SOURCES = microX86dis.c

libBinaryFile_la_SOURCES = BinaryFile.cpp SymTab.cpp

libBinaryFileFactory_la_SOURCES = BinaryFileFactory.cpp
libBinaryFileFactory_la_LIBADD = libBinaryFile.la -ldl

pkglib_LTLIBRARIES = \
	DOS4GWBinaryFile.la \
	ElfBinaryFile.la \
	ExeBinaryFile.la \
	HpSomBinaryFile.la \
	IntelCoffFile.la \
	MachOBinaryFile.la \
	PalmBinaryFile.la \
	Win32BinaryFile.la

DOS4GWBinaryFile_la_SOURCES = DOS4GWBinaryFile.cpp
ElfBinaryFile_la_SOURCES    = ElfBinaryFile.cpp
ExeBinaryFile_la_SOURCES    = ExeBinaryFile.cpp
HpSomBinaryFile_la_SOURCES  = HpSomBinaryFile.cpp
IntelCoffFile_la_SOURCES    = IntelCoffFile.cpp
MachOBinaryFile_la_SOURCES  = MachOBinaryFile.cpp
PalmBinaryFile_la_SOURCES   = PalmBinaryFile.cpp
Win32BinaryFile_la_SOURCES  = Win32BinaryFile.cpp

DOS4GWBinaryFile_la_LIBADD = libBinaryFile.la libmicroX86dis.la
ElfBinaryFile_la_LIBADD    = libBinaryFile.la
ExeBinaryFile_la_LIBADD    = libBinaryFile.la
HpSomBinaryFile_la_LIBADD  = libBinaryFile.la
IntelCoffFile_la_LIBADD    = libBinaryFile.la
MachOBinaryFile_la_LIBADD  = libBinaryFile.la
PalmBinaryFile_la_LIBADD   = libBinaryFile.la
Win32BinaryFile_la_LIBADD  = libBinaryFile.la libmicroX86dis.la

DOS4GWBinaryFile_la_LDFLAGS = -no-undefined -avoid-version -module -shared
ElfBinaryFile_la_LDFLAGS    = -no-undefined -avoid-version -module -shared
ExeBinaryFile_la_LDFLAGS    = -no-undefined -avoid-version -module -shared
HpSomBinaryFile_la_LDFLAGS  = -no-undefined -avoid-version -module -shared
IntelCoffFile_la_LDFLAGS    = -no-undefined -avoid-version -module -shared
MachOBinaryFile_la_LDFLAGS  = -no-undefined -avoid-version -module -shared
PalmBinaryFile_la_LDFLAGS   = -no-undefined -avoid-version -module -shared
Win32BinaryFile_la_LDFLAGS  = -no-undefined -avoid-version -module -shared

bin_PROGRAMS = \
	bffDump

bffDump_SOURCES = bffDump.cpp
bffDump_LDADD = libBinaryFileFactory.la

check_PROGRAMS = \
	testLoader

testLoader_SOURCES = testLoader.cpp LoaderTest.cpp
testLoader_CPPFLAGS = $(AM_CPPFLAGS) $(CPPUNIT_CFLAGS)
testLoader_LDADD = libBinaryFileFactory.la libmicroX86dis.la $(CPPUNIT_LIBS)

# FIXME: Find a cleaner way to point dlopen() to the not-yet-installed library files.
check-local: $(check_PROGRAMS) $(pkglib_LTLIBRARIES)
	LD_LIBRARY_PATH=".libs:$$LD_LIBRARY_PATH" ./testLoader
