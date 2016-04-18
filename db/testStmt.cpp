/*==============================================================================
 * FILE:       testStmt.cpp
 * OVERVIEW:   Command line test of the Statement class
 *============================================================================*/

#include "StatementTest.h"

#include <cppunit/ui/text/TestRunner.h>

#include <cstdlib>

int main(int argc, char *argv[])
{
	CppUnit::TextUi::TestRunner runner;

	runner.addTest(StatementTest::suite());

	return runner.run() ? EXIT_SUCCESS : EXIT_FAILURE;
}
