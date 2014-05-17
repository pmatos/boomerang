/*==============================================================================
 * FILE:       testExp.cc
 * OVERVIEW:   Command line test of the Exp and related classes.
 *============================================================================*/

#include "exp.h"

#include "ExpTest.h"

#include <cppunit/TextTestResult.h>
#include <cppunit/TestSuite.h>

#include <iostream>


int main(int argc, char** argv)
{
    CppUnit::TestSuite suite;

    ExpTest et("ExpTest");

    et.registerTests(&suite);

    CppUnit::TextTestResult res;

    suite.run( &res );
    std::cout << res << std::endl;

    return 0;
}

