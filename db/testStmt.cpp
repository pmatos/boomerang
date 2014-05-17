/*==============================================================================
 * FILE:	   testStmt.cpp
 * OVERVIEW:   Command line test of the Statement class
 *============================================================================*/

#include "StatementTest.h"

#include <cppunit/TextTestResult.h>
#include <cppunit/TestSuite.h>

#include <sstream>
#include <iostream>

int main(int argc, char** argv)
{
	CppUnit::TestSuite suite;

	StatementTest  expt("StatementTest");

	expt.registerTests(&suite);

	CppUnit::TextTestResult res;

	suite.run( &res );
	std::cout << res << std::endl;

	return 0;
}
