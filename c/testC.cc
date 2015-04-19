/*==============================================================================
 * FILE:       testC.cc
 * OVERVIEW:   Command line test of the C parser.
 *============================================================================*/

#include "CTest.h"

#include <cppunit/TextTestResult.h>
#include <cppunit/TestSuite.h>

#include <iostream>


int main(int argc, char **argv)
{
	CppUnit::TestSuite suite;

	CTest ct("C Parser Test");

	ct.registerTests(&suite);

	CppUnit::TextTestResult res;

	suite.run(&res);
	std::cout << res << std::endl;

	return 0;
}
