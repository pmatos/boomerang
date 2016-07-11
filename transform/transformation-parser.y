/**
 * \file
 * \brief Parser for transformation language.
 */

%name TransformationParser

%header{
	#include "exp.h"
	#include "type.h"
	#include "cfg.h"
	#include "proc.h"
	#include "signature.h"
	#include "transformer.h"
	#include "generic.h"

	class TransformationScanner;
%}

%union {
	int ival;
	const char *str;
	Type *type;
	Exp *exp;
}

%{
	#include "transformation-scanner.h"
%}

%define DEBUG 1

%define MEMBERS \
	private: \
		TransformationScanner *theScanner; \
	public: \
		virtual ~TransformationParser();

%define CONSTRUCTOR_PARAM std::istream &in, bool trace
%define CONSTRUCTOR_INIT
%define CONSTRUCTOR_CODE \
	theScanner = new TransformationScanner(in, trace); \
	if (trace) yydebug = 1; else yydebug = 0;

%define PARSE_PARAM

%token SIZEOF
%token KIND
%token POINTER COMPOUND ARRAY
%token TYPE
%token<str> FUNC
%token WHERE
%token BECOMES
%token REGOF
%token MEMOF
%token ADDROF
%token<ival> CONSTANT
%token<str> IDENTIFIER STRING_LITERAL
%token PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP
%token AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token XOR_ASSIGN OR_ASSIGN TYPE_NAME
%token STRUCT UNION ENUM ELLIPSIS
%token BOOL_TRUE BOOL_FALSE

%type<exp> exp
%type<exp> optional_where_clause
%type<type> type

%start translation_unit

%%

translation_unit
	: /* empty */
	| transformation translation_unit
	;

transformation
	: exp optional_where_clause BECOMES exp { new GenericExpTransformer($1, $2, $4); }
	;

optional_where_clause
	: /* empty */ { $$ = NULL; }
	| WHERE exp   { $$ = $2; }
	;

exp
	: REGOF CONSTANT ']' { $$ = Location::regOf($2); }
	| MEMOF exp ']'      { $$ = Location::memOf($2); }
	| ADDROF exp ']'     { $$ = new Unary(opAddrOf, $2); }
	| exp '+' exp        { $$ = new Binary(opPlus, $1, $3); }
	| exp '-' exp        { $$ = new Binary(opMinus, $1, $3); }
	| exp '*' exp        { $$ = new Binary(opMult, $1, $3); }
	| exp '&' exp        { $$ = new Binary(opBitAnd, $1, $3); }
	| exp '|' exp        { $$ = new Binary(opBitOr, $1, $3); }
	| exp '^' exp        { $$ = new Binary(opBitXor, $1, $3); }
	| exp '/' exp        { $$ = new Binary(opDiv, $1, $3); }
	| exp AND_OP exp     { $$ = new Binary(opAnd, $1, $3); }
	| exp OR_OP exp      { $$ = new Binary(opOr, $1, $3); }
	| exp EQ_OP exp      { $$ = new Binary(opEquals, $1, $3); }
	| exp NE_OP exp      { $$ = new Binary(opNotEqual, $1, $3); }
	| exp '.' exp        { $$ = new Binary(opMemberAccess, $1, $3); }
	| CONSTANT           { $$ = new Const($1); }
	| FUNC exp ')'       { $$ = new Binary(opFlagCall, new Const($1), $2); }
	| IDENTIFIER         {
		if ($1[0] == 'o' && $1[1] == 'p' && $1[2] != '\0')
			$$ = new Const($1); // treat op* as a string constant
		else
			$$ = new Unary(opVar, new Const($1));
		}
	| '(' exp ')'        { $$ = $2; }
	| KIND exp ')'       { $$ = new Unary(opKindOf, $2); }
	| TYPE exp ')'       { $$ = new Unary(opTypeOf, $2); }
	| '-' exp            { $$ = new Unary(opNeg, $2); }
	| '!' exp            { $$ = new Unary(opLNot, $2); }
	| type               { $$ = new TypeVal($1); }
	| exp ',' exp        { $$ = new Binary(opList, $1, new Binary(opList, $3, new Terminal(opNil))); }
	| BOOL_TRUE          { $$ = new Terminal(opTrue); }
	| BOOL_FALSE         { $$ = new Terminal(opFalse); }
	;

type
	: POINTER type ')' { $$ = new PointerType($2); }
	| COMPOUND         { $$ = new CompoundType(); }
	| IDENTIFIER       { $$ = new NamedType($1); }
	;

%%

#include <cstdlib>
#include <cstdio>
#include <cstring>

int TransformationParser::yylex()
{
	int token = theScanner->yylex(yylval);
	return token;
}

void TransformationParser::yyerror(const char *s)
{
	fprintf(stderr, "%s on line %i\n", s, theScanner->theLine);
	fprintf(stderr, "%s\n", theScanner->lineBuf.c_str());
	fprintf(stderr, "%*s\n", theScanner->column, "^");
}

TransformationParser::~TransformationParser()
{
	// Suppress warnings from gcc about lack of virtual destructor
}
