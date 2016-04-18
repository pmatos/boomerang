#include "BinaryFile.h"
#include "decoder.h"  // Actually use this class in the .cpp file

#include <cppunit/extensions/HelperMacros.h>

class FrontEnd;
class PentiumFrontEnd;

class FrontPentTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(FrontPentTest);
	CPPUNIT_TEST(test1);
	CPPUNIT_TEST(test2);
	CPPUNIT_TEST(test3);
	CPPUNIT_TEST(testBranch);
	CPPUNIT_TEST(testFindMain);
	CPPUNIT_TEST_SUITE_END();

public:
	FrontPentTest() { }

	void test1();
	void test2();
	void test3();
	void testBranch();
	void testFindMain();
};
