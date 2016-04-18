#include "prog.h"

#include <cppunit/extensions/HelperMacros.h>

class ProgTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(ProgTest);
	CPPUNIT_TEST(testName);
	CPPUNIT_TEST_SUITE_END();

protected:
	Prog *m_prog;

public:
	ProgTest() { }

	void setUp();

	void testName();
};
