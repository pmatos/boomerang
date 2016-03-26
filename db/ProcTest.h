/*==============================================================================
 * FILE:       ProcTest.h
 * OVERVIEW:   Provides the interface for the ProcTest class, which
 *              tests the Proc class
 *============================================================================*/

#include "proc.h"
#include "prog.h"

#include <cppunit/TestCaller.h>
#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>

class ProcTest : public CppUnit::TestCase {
protected:
	Proc *m_proc;

public:
	ProcTest(std::string name) : CppUnit::TestCase(name) { }

	virtual void registerTests(CppUnit::TestSuite *suite);

	int countTestCases() const;

	void setUp();
	void tearDown();

	void testName();
};
