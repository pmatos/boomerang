#include "util.h"
#include "type.h"

#include <cppunit/extensions/HelperMacros.h>

class TypeTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(TypeTest);
	CPPUNIT_TEST(testTypeLong);
	CPPUNIT_TEST(testNotEqual);
	CPPUNIT_TEST(testCompound);
	CPPUNIT_TEST(testDataInterval);
	CPPUNIT_TEST(testDataIntervalOverlaps);
	CPPUNIT_TEST_SUITE_END();

public:
	TypeTest() { }

	void testTypeLong();
	void testNotEqual();
	void testCompound();

	void testDataInterval();
	void testDataIntervalOverlaps();
};
