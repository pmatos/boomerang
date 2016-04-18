#include "util.h"
#include "type.h"

#include "ansi-c-parser.h"

#include <cppunit/extensions/HelperMacros.h>

class CTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(CTest);
	CPPUNIT_TEST(testSignature);
	CPPUNIT_TEST_SUITE_END();

public:
	CTest() { }

	void testSignature();
};
