/*==============================================================================
 * FILE:       testExp.cpp
 * OVERVIEW:   Command line test of the Exp class
 *============================================================================*/

#include "ExpTest.h"

#include <cppunit/TextTestResult.h>
#include <cppunit/TestSuite.h>

#include <sstream>
#include <iostream>

int main(int argc, char **argv)
{
	CppUnit::TestSuite suite;

	ExpTest expt("ExpTest");

	expt.registerTests(&suite);

	CppUnit::TextTestResult res;

	suite.run(&res);
	std::cout << res << std::endl;

	return 0;
}
