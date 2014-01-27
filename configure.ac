dnl See the file "LICENSE.TERMS" for information on usage and
dnl redistribution of this file, and for a DISCLAIMER OF ALL
dnl WARRANTIES.
dnl
dnl File: configure.in
dnl autoconf uses this file to create the configure script, which generates
dnl the Makefiles and sets up various things.

AC_PREREQ(2.61)
AC_INIT([boomerang],[1.0],[nietzsche@lysator.liu.se])
AC_CONFIG_SRCDIR([LICENSE.TERMS])
AC_CONFIG_HEADER(include/config.h)
AM_INIT_AUTOMAKE([foreign])

dnl Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_LIBTOOL

AC_PATH_PROG([BISONPP],[bison++])
AC_SUBST([BISONPP])
AC_PATH_PROG([FLEXPP],[flex++])
AC_SUBST([FLEXPP])
AC_PATH_PROG([M4],[m4])
AC_SUBST([M4])

PKG_PROG_PKG_CONFIG

PKG_CHECK_MODULES([EXPAT],[expat],
	[],
	[AC_MSG_ERROR([cannot find expat])])

PKG_CHECK_MODULES([GC],[bdw-gc],
	[],
	[AC_MSG_ERROR([cannot find gc])])

PKG_CHECK_MODULES([CPPUNIT],[cppunit],
	[],
	[AC_MSG_WARN([cannot find cppunit, needed only for testing])])


dnl Checks for header files.
dnl AC_HEADER_STDC
dnl AC_CHECK_HEADERS(fcntl.h malloc.h sys/time.h unistd.h byteswap.h)
dnl AC_CHECK_HEADERS(libelf.h libelf/libelf.h, break)
dnl AC_CHECK_HEADERS(link.h sys/link.h, break)
dnl AC_CHECK_HEADERS(elf.h sys/elf_SPARC.h sys/auxv.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_BIGENDIAN

AC_CHECK_SIZEOF(char)
AC_CHECK_SIZEOF(short)
AC_CHECK_SIZEOF(int)
AC_CHECK_SIZEOF(long)
AC_CHECK_SIZEOF(long long)
AC_CHECK_SIZEOF(float)
AC_CHECK_SIZEOF(double)
AC_CHECK_SIZEOF(int *)
dnl AC_CHECK_SIZEOF(long double)


dnl Checks for library functions.
AC_PROG_GCC_TRADITIONAL

dnl Now the options...
dnl AC_CHECK_FUNCS(mkdir rmdir)

dnl Three paths for frontend/mltk.sh. No longer give a "value-if-not-found", so can avoid errors when not installed
AC_PATH_PROG(TKML, .run-sml, , [$PATH /usr/share/sml-nj/bin $HOME/install/smlnj/bin])
dnl many more processors and platforms needed in this list
AC_SUBST(TKML)
AC_PATH_PROGS(HEAPPATH,
    [sml-toolkit.x86-linux sml-toolkit.sparc-solaris], ,
    [$HOME/install $PATH $HOME/install/mltk])
HEAP=\"@SMLload=$HEAPPATH\"
AC_SUBST(HEAP)

AC_PATH_PROG(UNGENERATE, ungenerate, , [$PATH:$HOME/install:$HOME/bin:/home/02/binary/u0.luna.tools/NJ/base])

PREFIX=$srcdir
LIBDIR=$srcdir/lib

AC_SUBST(PREFIX)
AC_SUBST(LIBDIR)


dnl only the makefile in the loader dir is currently used
AC_CONFIG_FILES(          Makefile)
AC_CONFIG_FILES(     util/Makefile)
AC_CONFIG_FILES(   loader/Makefile)
AC_CONFIG_FILES(       db/Makefile)
AC_CONFIG_FILES( frontend/Makefile)
AC_CONFIG_FILES(transform/Makefile)
AC_CONFIG_FILES(frontend/mltk.sh, [chmod +x frontend/mltk.sh])

AC_OUTPUT
