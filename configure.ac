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

dnl Checks for elf library (no longer needed)
dnl AC_CHECK_LIB(elf, elf_begin, , [AC_MSG_ERROR("library 'elf' not found")])

AC_CHECK_LIB(stdc++, main,, [AC_MSG_ERROR([library stdc++ is not found...])])

dnl Check for eXpat includes
dnl Want to check default-includes PLUS the Boomerang include/ directory, and some of the usual places it might already
dnl be installed
AC_CHECK_HEADER(expat.h,,
    [AC_CHECK_FILE(include/expat.h,,
		[AC_PATH_PROG(EXPAT_HDR_PATH, expat.h, :,
			[/usr/include:/usr/local/include:/opt/local/include:/usr/apache2/include:/opt/apache2/include:/sw/include])]
		if test "${EXPAT_HDR_PATH}" != :; then
			[AC_MSG_WARN(Found expat.h; linking to include/)]
			ln -s ${EXPAT_HDR_PATH} include
			dnl Sometimes, but not always, there is an expat_external.h as well
			[AC_PATH_PROG(EXPAT_HDR_PATH2, expat_external.h, :,
			[/usr/include:/usr/local/include:/opt/local/include:/usr/apache2/include:/opt/apache2/include:/sw/include])]
			if test "${EXPAT_HDR_PATH2}" != :; then
				[AC_MSG_WARN(Also found expat_eternal.h; linking to include/)]
				ln -s ${EXPAT_HDR_PATH2} include
			fi
        else
        	[AC_MSG_WARN()]
        	[AC_MSG_WARN(Header file expat.h not found: eXpat not installed...)]
        	[AC_MSG_WARN(or you need to copy expat.h to Boomerang's include/ dir)]
        	[AC_MSG_WARN()]
		fi
	)])

dnl Check for eXpat library itself
AC_CHECK_LIB(expat, XML_ExpatVersion, ,
    [AC_CHECK_FILE(lib/libexpat.a, ,
		[AC_PATH_PROG(EXPAT_PATH, libexpat.a, :,
			[/usr/lib:/usr/local/lib:/opt/local/lib:/usr/apache2/lib:/opt/apache2/lib:/sw/lib])]
		if test "${EXPAT_PATH}" != :; then
			[AC_MSG_WARN(Found libexpat.a; linking to lib/)]
			mkdir lib 2>/dev/null
			ln -s ${EXPAT_PATH} lib
        else
			[AC_MSG_WARN()]
			[AC_MSG_WARN(eXpat library not found: expat not installed... or you)]
			[AC_MSG_WARN(need to copy libexpat.a (etc) to Boomerang's lib/ dir)]
			[AC_MSG_WARN()]
		fi
	) ],)

dnl Check for gc (garbage collector) includes
dnl Want to check default-includes PLUS the Boomerang include/ directory, and some of the usual places it might already
dnl be installed
AC_CHECK_HEADER(gc.h,,
    [AC_CHECK_FILE(include/gc.h,,
		[AC_PATH_PROG(GC_HDR_PATH, gc.h, :, [/usr/include:/usr/include/gc:/usr/local/include:/opt/local/include:/sw/include])]
		if test "${GC_HDR_PATH}" != :; then
			[AC_MSG_WARN(Found gc.h; linking to include/)]
			ln -s ${GC_HDR_PATH} include
			dnl Sometimes, not sure if always, there is a gc_config_macros.h as well
			[AC_PATH_PROG(GC_HDR_PATH2, gc_config_macros.h, :,
			[/usr/include:/usr/include/gc:/usr/local/include:/opt/local/include:/sw/include])]
			if test "${GC_HDR_PATH2}" != :; then
				[AC_MSG_WARN(Also found gc_config_macros.h; linking to include/)]
				ln -s ${GC_HDR_PATH2} include
			fi
        else
        	[AC_MSG_WARN()]
        	[AC_MSG_WARN(Header file gc.h not found: Boehm garbage collector likely not installed...)]
        	[AC_MSG_WARN(or you need to copy gc.h and gc_config_macros.h to Boomerang's include/ dir)]
        	[AC_MSG_WARN()]
		fi
	)])

dnl Check for gc library itself
AC_CHECK_LIB(gc, GC_malloc, ,
    [AC_CHECK_FILE(lib/libgc.a, ,
		[AC_PATH_PROG(GC_PATH, libgc.a, :, [/usr/lib:/usr/local/lib:/opt/local/lib:/sw/lib])]
		if test "${GC_PATH}" != :; then
			[AC_MSG_WARN(Found libgc.a; linking to lib/)]
			mkdir lib 2>/dev/null
			ln -s ${GC_PATH} lib
        else
			[AC_MSG_WARN()]
			[AC_MSG_WARN(Boehm garbage collector library not found: not installed...)]
			[AC_MSG_WARN(or you need to copy libgc.a (etc) to Boomerang's lib/ dir)]
			[AC_MSG_WARN()]
		fi
	) ],)

dnl Check for dlfcn.h (include for dlopen etc)
dnl Want to check default-includes PLUS the Boomerang include/ directory, and some of the usual places it might already
dnl be installed
AC_CHECK_HEADER(dlfcn.h,,
    [AC_CHECK_FILE(include/dlfcn.h,,
		[AC_PATH_PROG(DL_HDR_PATH, dlfcn.h, :, [/usr/include:/usr/local/include:/opt/local/include:/sw/include])]
		if test "${DL_HDR_PATH}" != :; then
			[AC_MSG_WARN(Found dlfcn.h; linking to include/)]
			ln -s ${DL_HDR_PATH} include
        else
        	[AC_MSG_WARN()]
        	[AC_MSG_WARN(Header file dlfcn.h not found; needed for dynamic library file access.)]
        	[AC_MSG_WARN(You probably need to copy dlfcn.h to Boomerang's include/ dir)]
        	[AC_MSG_WARN()]
		fi
	)])

dnl Check for libdl (dynamic linked library loading and calling) library
AC_CHECK_LIB(dl, dlopen, ,
    [AC_CHECK_FILE(lib/libdl.a, ,
		[AC_PATH_PROG(DL_PATH, libdl.a, :, [/usr/lib:/usr/local/lib:/opt/local/lib:/sw/lib])]
		if test "${DL_PATH}" != :; then
			[AC_MSG_WARN(Found libdl.a; linking to lib/)]
			mkdir lib 2>/dev/null
			ln -s ${DL_PATH} lib
        else
			[AC_MSG_WARN()]
        	[AC_MSG_WARN(dl (dynamically linked library loading and calling) library not found)]
        	[AC_MSG_WARN(you need to copy libdl.a (etc) to Boomerang's lib/ dir)]
			[AC_MSG_WARN()]
		fi
	) ],)

dnl Checks for optional cppunit library
AC_LANG_PUSH(C++)
CPPFLAGS="-I\$(top_srcdir)/include ${CPPFLAGS}"
AC_CHECK_HEADER(cppunit/TestSuite.h,,
    [AC_CHECK_FILE(include/cppunit/TestSuite.h,,
		[AC_PATH_PROG(CPPUNIT_HDR_PATH, cppunit/TestSuite.h, :,
			[/usr/include:/usr/local/include:/opt/local/include:/sw/include])]
		if test "${CPPUNIT_HDR_PATH}" != :; then
			[AC_MSG_WARN(Found cppunit include directory; linking to include/)]
			dnl Link the whole include directory
			dnl Note: I can't get AS_DIRNAME() to work...
			cpp_path=`dirname $CPPUNIT_HDR_PATH`
			ln -s ${cpp_path} include
        else
        	[AC_MSG_NOTICE()]
        	[AC_MSG_NOTICE(Header directories for cppunit not found; needed only for unit testing.)]
        	[AC_MSG_NOTICE()]
		fi
	)])

dnl Check for cppunut (C++ unit testing) library
AC_CHECK_LIB(cppunit, _ZN7CppUnit8TestCase3runEv, ,
    [AC_CHECK_FILE(lib/libcppunit.a, ,
		[AC_PATH_PROG(CPPUNIT_PATH, libcppunit.a, :, [/usr/lib:/usr/local/lib:/opt/local/lib:/sw/lib])]
		if test "${CPPUNIT_PATH}" != :; then
			[AC_MSG_WARN(Found libcppunit.a; linking to lib/)]
			mkdir lib 2>/dev/null
			ln -s ${CPPUNIT_PATH} lib
        else
			[AC_MSG_NOTICE()]
        	[AC_MSG_NOTICE(cppunit (C++ Unit Testing) library not found; needed only for testing)]
			[AC_MSG_NOTICE()]
		fi
	) ],)

AC_LANG_POP(C++)




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

dnl Actually we have "make remote" again now, so this is unused at present:
AC_ARG_ENABLE(remote, [  --enable-remote         don't try to regenerate source files],
              [ REMOTE=1 ], [ REMOTE=0 ] )
AC_SUBST(REMOTE)

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
