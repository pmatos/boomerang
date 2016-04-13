/*==============================================================================
 * FILE:       testRtl.cpp
 * OVERVIEW:   Command line test of the Rtl class
 *============================================================================*/

#include "RtlTest.h"

#include <cppunit/TextTestResult.h>
#include <cppunit/TestSuite.h>

#include <sstream>
#include <iostream>

int main(int argc, char *argv[])
{
	CppUnit::TestSuite suite;

	RtlTest expt("RtlTest");

	expt.registerTests(&suite);

	CppUnit::TextTestResult res;

	suite.run(&res);
	std::cout << res << std::endl;

	return 0;
}
