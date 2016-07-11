#include "prog.h"

#include <cppunit/extensions/HelperMacros.h>

class ParserTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(ParserTest);
	CPPUNIT_TEST(testRead);
	CPPUNIT_TEST(testExp);
	CPPUNIT_TEST_SUITE_END();

public:
	ParserTest() { }

	void testRead();
	void testExp();
};
