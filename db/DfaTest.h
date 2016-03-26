#include "type.h"

#include <cppunit/TestCaller.h>
#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>

class DfaTest : public CppUnit::TestCase {
protected:

public:
	DfaTest(std::string name) : CppUnit::TestCase(name) { }

	virtual void registerTests(CppUnit::TestSuite *suite);

	int countTestCases() const;

	void setUp();
	void tearDown();

	void testMeetInt();
	void testMeetSize();
	void testMeetPointer();
	void testMeetUnion();
};
