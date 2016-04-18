#include "type.h"

#include <cppunit/extensions/HelperMacros.h>

class DfaTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(DfaTest);
	CPPUNIT_TEST(testMeetInt);
	CPPUNIT_TEST(testMeetSize);
	CPPUNIT_TEST(testMeetPointer);
	CPPUNIT_TEST(testMeetUnion);
	CPPUNIT_TEST_SUITE_END();

public:
	DfaTest() { }

	void testMeetInt();
	void testMeetSize();
	void testMeetPointer();
	void testMeetUnion();
};
