/**
 * \file
 * \brief Command line test of all of Boomerang.
 */

#include "exp.h"

#include "ExpTest.h"
#include "ProgTest.h"
#include "ProcTest.h"
#include "StatementTest.h"
#include "RtlTest.h"
#include "DfaTest.h"
#include "ParserTest.h"
#include "TypeTest.h"
#include "FrontSparcTest.h"
#include "FrontPentTest.h"
#include "CTest.h"
#include "CfgTest.h"

#include "prog.h"

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
	runner.addTest(FrontSparcTest::suite());
	runner.addTest(FrontPentTest::suite());
	runner.addTest(CTest::suite());
	runner.addTest(StatementTest::suite());
	runner.addTest(CfgTest::suite());
	runner.addTest(DfaTest::suite());

	return runner.run() ? EXIT_SUCCESS : EXIT_FAILURE;
}
