/**
 * \file
 * \brief Command line test of the Rtl class.
 */

#include "RtlTest.h"

#include <cppunit/ui/text/TestRunner.h>

#include <cstdlib>

int main(int argc, char *argv[])
{
	CppUnit::TextUi::TestRunner runner;

	runner.addTest(RtlTest::suite());

	return runner.run() ? EXIT_SUCCESS : EXIT_FAILURE;
}
