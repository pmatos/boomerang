/*==============================================================================
 * FILE:       testLoader.cc
 * OVERVIEW:   Command line test of the BinaryFile and related classes.
 *============================================================================*/

#include "cppunit/TextTestResult.h"
#include "cppunit/TestSuite.h"

#include <iostream>
#include "LoaderTest.h"

int main(int argc, char** argv)
{
    CppUnit::TestSuite suite;

    LoaderTest lt("ExpTest");

    lt.registerTests(&suite);

    CppUnit::TextTestResult res;

    suite.run( &res );
    std::cout << res << std::endl;

    return 0;
}

