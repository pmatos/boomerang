#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([Boomerang],[0.3.1])
AC_CONFIG_SRCDIR([boomerang.cpp])
AC_CONFIG_HEADERS([include/config.h])
AM_INIT_AUTOMAKE([foreign])
LT_INIT([dlopen])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
PKG_PROG_PKG_CONFIG

AC_PATH_PROG([BISONPP],[bison++])
AC_SUBST([BISONPP])
AC_PATH_PROG([FLEXPP],[flex++])
AC_SUBST([FLEXPP])
AC_PATH_PROG([M4],[m4])
AC_SUBST([M4])

# Three paths for frontend/mltk.sh. No longer give a "value-if-not-found", so can avoid errors when not installed
AC_PATH_PROG([TKML],[.run-sml],,
	[$PATH /usr/share/sml-nj/bin $HOME/install/smlnj/bin])
AC_SUBST([TKML])

# many more processors and platforms needed in this list
AC_PATH_PROGS([HEAPPATH],
	[sml-toolkit.x86-linux sml-toolkit.sparc-solaris],,
	[$HOME/install $PATH $HOME/install/mltk])
HEAP=\"@SMLload=$HEAPPATH\"
AC_SUBST([HEAP])

AC_PATH_PROG([UNGENERATE],[ungenerate],,
	[$PATH:$HOME/install:$HOME/bin:/home/02/binary/u0.luna.tools/NJ/base])

# Checks for libraries.
PKG_CHECK_MODULES([EXPAT],[expat],
	[],
	[AC_MSG_ERROR([cannot find expat])])

PKG_CHECK_MODULES([GC],[bdw-gc],
	[],
	[AC_MSG_ERROR([cannot find gc])])

PKG_CHECK_MODULES([CPPUNIT],[cppunit],
	[],
	[AC_MSG_WARN([cannot find cppunit, needed only for testing])])

# Checks for header files.
#AC_HEADER_STDC
#AC_CHECK_HEADERS(fcntl.h malloc.h sys/time.h unistd.h byteswap.h)
#AC_CHECK_HEADERS(libelf.h libelf/libelf.h, break)
#AC_CHECK_HEADERS(link.h sys/link.h, break)
#AC_CHECK_HEADERS(elf.h sys/elf_SPARC.h sys/auxv.h)

# Checks for typedefs, structures, and compiler characteristics.
AC_C_BIGENDIAN

# Checks for library functions.
#AC_CHECK_FUNCS(mkdir rmdir)

AC_CONFIG_FILES([
	Makefile
	c/Makefile
	codegen/Makefile
	db/Makefile
	frontend/Makefile
	loader/Makefile
	transform/Makefile
	type/Makefile
	util/Makefile
])
AC_CONFIG_FILES([frontend/mltk.sh],[chmod +x frontend/mltk.sh])

AC_OUTPUT
