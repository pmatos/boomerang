/*==============================================================================
 * FILE:       testFront.cc
 * OVERVIEW:   Command line test of the Frontend and related classes.
 *============================================================================*/

//#include "FrontSparcTest.h"
#include "FrontPentTest.h"
//#include "FrontendTest.h"
#include "prog.h"

#include <cppunit/TextTestResult.h>
#include <cppunit/TestSuite.h>

#include <iostream>

int main(int argc, char *argv[])
{
	CppUnit::TestSuite suite;

	//FrontSparcTest fst("FrontSparcTest");
	//FrontendTest fet("FrontendTest");
	FrontPentTest fpt("FrontPentTest");

	//fst.registerTests(&suite);
	fpt.registerTests(&suite);

	CppUnit::TextTestResult res;

	prog.readLibParams();  // Read library signatures (once!)
	suite.run(&res);
	std::cout << res << std::endl;

	return 0;
}
