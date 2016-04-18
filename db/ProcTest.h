/*==============================================================================
 * FILE:       ProcTest.h
 * OVERVIEW:   Provides the interface for the ProcTest class, which
 *              tests the Proc class
 *============================================================================*/

#include "proc.h"
#include "prog.h"

#include <cppunit/extensions/HelperMacros.h>

class ProcTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(ProcTest);
	CPPUNIT_TEST(testName);
	CPPUNIT_TEST_SUITE_END();

protected:
	Proc *m_proc;

public:
	ProcTest() { }

	void tearDown();

	void testName();
};
