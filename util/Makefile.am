AM_CPPFLAGS = \
	-I$(top_srcdir)/include


noinst_LTLIBRARIES = \
	libutil.la

libutil_la_SOURCES = \
	util.cpp


check_LTLIBRARIES = \
	libUtilTest.la

libUtilTest_la_SOURCES = \
	UtilTest.cpp \
	UtilTest.h
libUtilTest_la_LIBADD = \
	$(CPPUNIT_LIBS)
libUtilTest_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	$(CPPUNIT_CFLAGS)
