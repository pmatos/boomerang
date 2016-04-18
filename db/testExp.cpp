/*==============================================================================
 * FILE:       testExp.cpp
 * OVERVIEW:   Command line test of the Exp class
 *============================================================================*/

#include "ExpTest.h"

#include <cppunit/ui/text/TestRunner.h>

#include <cstdlib>

int main(int argc, char *argv[])
{
	CppUnit::TextUi::TestRunner runner;

	runner.addTest(ExpTest::suite());

	return runner.run() ? EXIT_SUCCESS : EXIT_FAILURE;
}
