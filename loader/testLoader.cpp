/*==============================================================================
 * FILE:       testLoader.cc
 * OVERVIEW:   Command line test of the BinaryFile and related classes.
 *============================================================================*/

#include "LoaderTest.h"

#include <cppunit/ui/text/TestRunner.h>

#include <cstdlib>

int main(int argc, char *argv[])
{
	CppUnit::TextUi::TestRunner runner;

	runner.addTest(LoaderTest::suite());

	return runner.run() ? EXIT_SUCCESS : EXIT_FAILURE;
}
