/**
 * \file
 * \brief Defines a scanner to parse the tokens of a Semantic Specification
 *        Language specification.
 *
 * \authors
 * Copyright (C) 1997, Shane Sendall
 * \authors
 * Copyright (C) 1998-2001, The University of Queensland
 * \authors
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * \copyright
 * See the file "LICENSE.TERMS" for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */

%name SSLScanner

%header{
	#include "sslparser.h"
	#define MAX_LINE 1024      // the longest SSL spec line
%}

%{
#include <cstdlib>
#include <cstring>
%}

%define IOSTREAM

%define MEMBERS \
	public: \
		int theLine;            /* the current line number */ \
		char lineBuf[MAX_LINE]; /* the current line */ \
		bool traceLines;        /* echo each lines as it is scanned */ \

%define CONSTRUCTOR_PARAM std::istream &ins, bool trace
%define CONSTRUCTOR_INIT : theLine(1), traceLines(trace)
%define CONSTRUCTOR_CODE yyin = &ins;

%define LEX_PARAM YY_SSLParser_STYPE &yylval

%%

"INTEGER"    |
"integer"    { return SSLParser::TOK_INTEGER; }
"FLOAT"      |
"float"      { return SSLParser::TOK_FLOAT; }
"OPERAND"    { return SSLParser::OPERAND; }
"ENDIANNESS" |
"endianness" { return SSLParser::ENDIANNESS; }
"BIG"        |
"big"        { return SSLParser::BIG; }
"LITTLE"     |
"little"     { return SSLParser::LITTLE; }
"COVERS"     { return SSLParser::COVERS; }
"SHARES"     { return SSLParser::SHARES; }
"FAST"       { return SSLParser::FAST; }
"FETCHEXEC"  { return SSLParser::FETCHEXEC; }

"*"[a-z]?[0-9]*"*" { yylval.str = strdup(yytext); return SSLParser::ASSIGNTYPE; }

"or"    |
"and"   { yylval.str = strdup(yytext); return SSLParser::LOG_OP; }

"="     |
"~="    |
"<"     |
">"     |
"<="    |
">="    |
"<u"    |
">u"    |
"<=u"   |
">=u"   { yylval.str = strdup(yytext); return SSLParser::COND_OP; }

"rlc"   |
"rrc"   |
"rl"    |
"rr"    |
">>"    |
"<<"    |
">>A"   |
"|"     |
"&"     |
"^"     { yylval.str = strdup(yytext); return SSLParser::BIT_OP; }

"%"     |
"*"     |
"/"     |
"*!"    |
"/!"    |
"%!"    |
"+"     |
"-"     { yylval.str = strdup(yytext); return SSLParser::ARITH_OP; }

"*f"    |
"*fd"   |
"*fq"   |
"*fsd"  |
"*fdq"  |
"/f"    |
"/fd"   |
"/fq"   |
"+f"    |
"+fd"   |
"+fq"   |
"-f"    |
"-fd"   |
"-fq"   |
"pow"   { yylval.str = strdup(yytext); return SSLParser::FARITH_OP; }

"~"     { yylval.op = opNot;  return SSLParser::NOT; }
"L~"    { yylval.op = opLNot; return SSLParser::LNOT; }
"~f"    { yylval.op = opFNeg; return SSLParser::FNEG; }
"=>"    { return SSLParser::THEN; }
"->"    { return SSLParser::INDEX; }
":="    { return SSLParser::EQUATE; }
"::="   { return SSLParser::ASSIGN; }
".."    { return SSLParser::TO; }
"!"     { yylval.op = opSignExt; return SSLParser::S_E; }
"@"     { return SSLParser::AT; }

"addr("          { yylval.op = opAddrOf;  return SSLParser::ADDR; }
"fsize("         { yylval.op = opFsize;   return SSLParser::CONV_FUNC; }
"itof("          { yylval.op = opItof;    return SSLParser::CONV_FUNC; }
"ftoi("          { yylval.op = opFtoi;    return SSLParser::CONV_FUNC; }
"fround("        { yylval.op = opFround;  return SSLParser::CONV_FUNC; }
"truncu("        { yylval.op = opTruncu;  return SSLParser::CONV_FUNC; }
"truncs("        { yylval.op = opTruncs;  return SSLParser::CONV_FUNC; }
"zfill("         { yylval.op = opZfill;   return SSLParser::CONV_FUNC; }
"sgnex("         { yylval.op = opSgnEx;   return SSLParser::CONV_FUNC; }
"ftrunc("        { yylval.op = opFtrunc;  return SSLParser::TRUNC_FUNC; }
"fabs("          { yylval.op = opFabs;    return SSLParser::FABS_FUNC; }
"defineflags("   |
"undefineflags(" { return SSLParser::FLAGMACRO; }
"FPOP"           { yylval.op = opFpop;    return SSLParser::FPOP; }
"FPUSH"          { yylval.op = opFpush;   return SSLParser::FPUSH; }
"sin("           { yylval.op = opSin;     return SSLParser::TRANSCEND; }
"cos("           { yylval.op = opCos;     return SSLParser::TRANSCEND; }
"tan("           { yylval.op = opTan;     return SSLParser::TRANSCEND; }
"arctan("        { yylval.op = opArcTan;  return SSLParser::TRANSCEND; }
"log2("          { yylval.op = opLog2;    return SSLParser::TRANSCEND; }
"loge("          { yylval.op = opLoge;    return SSLParser::TRANSCEND; }
"log10("         { yylval.op = opLog10;   return SSLParser::TRANSCEND; }
"execute("       { yylval.op = opExecute; return SSLParser::TRANSCEND; }
"sqrt("          { yylval.op = opSqrt;    return SSLParser::TRANSCEND; }
"succ("          { return SSLParser::SUCCESSOR; }

"tmp"[a-zA-Z0-9_]* { yylval.str = strdup(yytext); return SSLParser::TEMP; }

"r["        { return SSLParser::REG_IDX; }
"r"[0-9]*   { yylval.num = strtol(&yytext[1], NULL, 10); return SSLParser::REG_NUM; }
"m["        { return SSLParser::MEM_IDX; }
"_"         { return yytext[0]; }

"%"[A-Za-z][A-Za-z0-9]*    { yylval.str = strdup(yytext); return SSLParser::REG_ID; }
_?[A-Za-z][A-Za-z0-9_]*"(" { yytext[--yyleng] = '\0'; yylval.str = strdup(yytext); return SSLParser::NAME_CALL; }
_?[A-Za-z][A-Za-z0-9_]*"[" { yytext[--yyleng] = '\0'; yylval.str = strdup(yytext); return SSLParser::NAME_LOOKUP; }
_?[A-Za-z][A-Za-z0-9_]*    { yylval.str = strdup(yytext); return SSLParser::NAME; }
"."[A-Za-z][A-Za-z.0-9]*   |
"^"\"[A-Za-z][A-Za-z]*\"   { yylval.str = strdup(yytext); return SSLParser::DECOR; }

[?:,{}()'"] |
"]"         |
"["         |
";"         |
"$"         { return yytext[0]; }

-?[0-9]+"."[0-9]+ { yylval.dbl = atof(yytext); return SSLParser::FLOATNUM; }
-?[0-9]+          { yylval.num = strtol(yytext, NULL, 10); return SSLParser::NUM; }
0x[A-F0-9]+       { yylval.num = strtol(yytext, NULL, 16); return SSLParser::NUM; }

-?"2**"[0-9]+ { // a power of two
	int sign = (yytext[0] == '-' ? -1 : 1);
	char *start = yytext + (sign == -1 ? 4 : 3);

	// David - changed to << for more efficient and
	// not depend on the <cmath> anymore.
	//yylval.num = (int)pow(2, strtol(start, NULL, 10)) * sign;
	yylval.num = 1 << (strtol(start, NULL, 10) * sign);
	return SSLParser::NUM;
}

[ \t]+  ;
#.*$    ; /* comment */
.       ;
\n.*    { // funky print-next-line-for-debug token
	if (yyleng > MAX_LINE) {
		std::cerr << theLine + 1 << ": line too long.\n";
		exit(1);
	}
	// saves the next line
	strcpy(lineBuf, yytext + 1);
	theLine++;
	// reparse from char 1
	// give back all but the \n to rescan
	yyless(1);

	// echo the line if line tracing is turned on
	if (traceLines)
	std::cerr << lineBuf << std::endl;

	// Quell a warning
	yy_last_accepting_state = 0;
	yy_last_accepting_cpos = 0;
}

%%
