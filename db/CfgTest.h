#include "cfg.h"

#include <cppunit/extensions/HelperMacros.h>

class CfgTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(CfgTest);
	// Oops - they were all for dataflow. Need some real Cfg tests!
	CPPUNIT_TEST(testDominators);
	CPPUNIT_TEST(testSemiDominators);
	//CPPUNIT_TEST(testPlacePhi);
	//CPPUNIT_TEST(testPlacePhi2);
	CPPUNIT_TEST(testRenameVars);
	CPPUNIT_TEST_SUITE_END();

protected:
	Cfg *m_prog;

public:
	CfgTest() { }

	void setUp();

	void testDominators();
	void testSemiDominators();
	void testPlacePhi();
	void testPlacePhi2();
	void testRenameVars();
};
