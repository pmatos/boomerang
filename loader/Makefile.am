######################################################
# File: Makefile
#
# Desc: Makefile for the Loader library files.
#   This loader is lightly adapted from the loader in UQBT
#   Purpose is to load the input binary file into memory, with calls to iterate through sections,
#	find entry points, symbols, etc.
#   The main library is BinaryFile, which calls eactly one of ElfBinaryFile etc as needed.
#
######################################################

####################
# Compiler settings
#

FPIC = -fPIC

####################################
# Detection of OS specific settings
#

# LDL is -ldl if needed (Windows does not)
#  is .exe for Windows
# DLLEXT is .dll for Windows and .so otherwise 
if HOST_IS_WINDOWS
 LDL=
 DLLEXT=.dll
 WININCLUDE=-Iwin32make/include
 # RUNPATH is not needed on Windows 
 RUNPATH=
 COPYDLL=$(top_srcdir)/libBinaryFile$(DLLEXT)
else # Non-Windows
 LDL=-ldl
 DLLEXT=.so
 WININCLUDE=
if HOST_IS_GNU_LD
  RUNPATH=-Wl,-rpath -Wl,"lib"
else
if HOST_IS_OSX
   RUNPATH=
else		# Assume Solaris
   RUNPATH=-R"lib"
endif
endif
 COPYDLL=
endif

if HOST_IS_OSX
if HOST_IS_OSX_10_2
  SHARED=-dynamiclib		# Note: doesn't seem to work at present; fiddle manually
else
  SHARED=-dynamic -dylib
endif
 #-flat_namespace
 DLLEXT=.dylib
 RUNPATH=
else # Non-OS X
 SHARED=-shared
endif

######################
# Rules for compiling
#

LOADERS = \
	ElfBinaryFile.cpp \
	Win32BinaryFile.cpp \
	ExeBinaryFile.cpp \
	HpSomBinaryFile.cpp \
	PalmBinaryFile \
	DOS4GWBinaryFile.cpp \
	IntelCoffFile.cpp \
	$(NULL)

pkglib_LTLIBRARIES = 

if HOST_IS_CYGWIN
 # Until we can resolve the issue of compiling loader/MachOBinaryFile.cpp, don't attempt to make it so at least
 # the Cygwin hosted Boomerang can compile and use non-MachO loaders
 LOADERS+=MachOBinaryFile.cpp
endif
#LOADERDLLS=$(patsubst %,$(top_srcdir)/lib/lib%$(DLLEXT),$(LOADERS))
#BASEDLL=$(top_srcdir)/lib/libBinaryFile$(DLLEXT)
#ALLDLLS=$(LOADERDLLS) $(BASEDLL)
ALLLIBS=$(patsubst %,-l%,$(LOADERS)) -lBinaryFile
ALLOBJS = $(patsubst %, %.o, $(LOADERS))
ALLOBJS += BinaryFile.o SymTab.o

all:    $(ALLDLLS) $(COPYDLL) BinaryFileFactory.o BinaryFileStub.o

microX86dis.o: microX86dis.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $(FPIC) $<
BinaryFileFactory.o:
BinaryFileStub.o:

# Extra object files, etc
# These are target specific variables.
$(top_srcdir)/lib/libWin32BinaryFile$(DLLEXT): EXTRAS = microX86dis.o
$(top_srcdir)/lib/libWin32BinaryFile$(DLLEXT): microX86dis.o
$(top_srcdir)/lib/libDOS4GWBinaryFile$(DLLEXT): EXTRAS = microX86dis.o
$(top_srcdir)/lib/libDOS4GWBinaryFile$(DLLEXT): microX86dis.o

$(top_srcdir)/libBinaryFile$(DLLEXT): $(top_srcdir)/lib/libBinaryFile$(DLLEXT)
	cp $< $@

BinaryFile.o:		CCEXTRAS = -DBUILDING_LIBBINARYFILE
MachOBinaryFile.o:	CCEXTRAS = -I.

$(BASEDLL) : BinaryFile.o SymTab.o
	$(CXX) $(CXXFLAGS) -o $@ $(SHARED) BinaryFile.o SymTab.o $(OUTIMPLIB) $(LDFLAGS)

# This pattern generates all the main dependencies
$(LOADERDLLS): $(top_srcdir)/lib/lib%$(DLLEXT) : %.o $(BASEDLL)
	$(CXX) $(CXXFLAGS) -o $@ $(SHARED) $< SymTab.o $(EXTRAS) -lBinaryFile $(RUNPATH) -L$(top_srcdir)/lib $(LDFLAGS)

# Compile all objects with -fPIC
$(ALLOBJS): %.o : %.cpp
	$(CXX) $(CXXFLAGS) -c $(FPIC) $(CCEXTRAS) -I$(top_srcdir)/include -o $@ $<

###############
# Unit testing
#

testLoader: testLoader.o LoaderTest.o microX86dis.o BinaryFileFactory.o SymTab.o
	$(CXX) $(CXXFLAGS) -o $@ $^  -lcppunit $(ALLLIBS) $(RUNPATH) $(LDL) -L$(top_srcdir)/lib $(LDFLAGS)

testLoader.o: testLoader.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ -I$(top_srcdir)/include $<

LoaderTest.o: LoaderTest.cpp LoaderTest.h $(top_srcdir)/include/BinaryFile.h SymTab.o
	$(CXX) $(CXXFLAGS) -c -o $@ -I$(top_srcdir)/include $<

# Because of the relative paths, testLoader will only work from the parent
# directory...
test: all testLoader
	cd $(top_srcdir) && loader/testLoader

