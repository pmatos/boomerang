#include "BinaryFile.h"

#include <cppunit/extensions/HelperMacros.h>

class FrontendTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(FrontendTest);
	CPPUNIT_TEST(test1);
	CPPUNIT_TEST_SUITE_END();

public:
	FrontendTest() { }

	void test1();
};
