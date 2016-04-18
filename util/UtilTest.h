#include "util.h"
#include "type.h"

#include <cppunit/extensions/HelperMacros.h>

class UtilTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(UtilTest);
	//CPPUNIT_TEST(testTypeLong);
	//CPPUNIT_TEST(testNotEqual);
	CPPUNIT_TEST_SUITE_END();

public:
	UtilTest() { }

	void testTypeLong();
	void testNotEqual();
};
