/**
 * \file
 * \brief Command line test of the Exp and related classes.
 */

#include "exp.h"

#include "ExpTest.h"
#include "ProgTest.h"
#include "ProcTest.h"
#include "RtlTest.h"
#include "ParserTest.h"
#include "TypeTest.h"

#include <cppunit/ui/text/TestRunner.h>

#include <cstdlib>

int main(int argc, char *argv[])
{
	CppUnit::TextUi::TestRunner runner;

	runner.addTest(ExpTest::suite());
	runner.addTest(ProgTest::suite());
	runner.addTest(ProcTest::suite());
	runner.addTest(RtlTest::suite());
	runner.addTest(ParserTest::suite());
	runner.addTest(TypeTest::suite());

	return runner.run() ? EXIT_SUCCESS : EXIT_FAILURE;
}
