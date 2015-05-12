#include "BinaryFile.h"

#include <cppunit/TestCaller.h>
#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>

class FrontendTest : public CppUnit::TestCase {
protected:

public:
	FrontendTest(std::string name) : CppUnit::TestCase(name) { }

	virtual void registerTests(CppUnit::TestSuite *suite);

	int countTestCases() const;

	void setUp();
	void tearDown();

	void test1();
};
