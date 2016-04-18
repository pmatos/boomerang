/*==============================================================================
 * FILE:       ParserTest.cc
 * OVERVIEW:   Provides the implementation for the ParserTest class, which
 *              tests the sslparser.y etc
 *============================================================================*/

#define SPARC_SSL       "frontend/machine/sparc/sparc.ssl"

#include "ParserTest.h"
#include "sslparser.h"

#include <sstream>

/*==============================================================================
 * FUNCTION:        ParserTest::testRead
 * OVERVIEW:        Test reading the SSL file
 *============================================================================*/
void ParserTest::testRead()
{
	RTLInstDict d;
	CPPUNIT_ASSERT(d.readSSLFile(SPARC_SSL));
}

/*==============================================================================
 * FUNCTION:        ParserTest::testExp
 * OVERVIEW:        Test parsing an expression
 *============================================================================*/
void ParserTest::testExp()
{
	std::string s("*i32* r0 := 5 + 6");
	Statement *a = SSLParser::parseExp(s.c_str());
	CPPUNIT_ASSERT(a);
	std::ostringstream ost;
	a->print(ost);
	CPPUNIT_ASSERT_EQUAL("   0 " + s, std::string(ost.str()));
	std::string s2 = "*i32* r[0] := 5 + 6";
	a = SSLParser::parseExp(s2.c_str());
	CPPUNIT_ASSERT(a);
	std::ostringstream ost2;
	a->print(ost2);
	// Still should print to string s, not s2
	CPPUNIT_ASSERT_EQUAL("   0 " + s, std::string(ost2.str()));
}
