/**
 * \file
 * \brief Scanner for transformation language.
 */

%name TransformationScanner

%header{
	#include "transformation-parser.h"
	#include <string>
%}

%{
	#include <string.h>
	#define YY_USER_ACTION \
		count();
%}

%define IOSTREAM

%define MEMBERS \
	public: \
		int theLine;        /* the current line number */ \
		std::string lineBuf; /* the current line */ \
		bool traceLines;    /* echo each line as it is scanned */ \
		int column; \
	private: \
		void count();

%define CONSTRUCTOR_PARAM std::istream &ins, bool trace
%define CONSTRUCTOR_INIT : theLine(1), traceLines(trace), column(0)
#define CONSTRUCTOR_CODE yyin = &ins;

%define LEX_PARAM YY_TransformationParser_STYPE &yylval


D                       [0-9]
/* Allow ? and @ in identifiers, so we can use mangled names */
L                       [a-zA-Z_?@]
H                       [a-fA-F0-9]
E                       [Ee][+-]?{D}+
FS                      (f|F|l|L)
IS                      (u|U|l|L)*

%x COMMENT

%%

"/*"                    { BEGIN(COMMENT); }
<COMMENT>"*/"           { BEGIN(INITIAL); }
<COMMENT>[^*\n]+        ;
<COMMENT>"*"            ;
<COMMENT>\n             ;

"sizeof"                { return TransformationParser::SIZEOF; }
"where"                 { return TransformationParser::WHERE; }
"becomes"               { return TransformationParser::BECOMES; }
"kind("                 { return TransformationParser::KIND; }
"type("                 { return TransformationParser::TYPE; }
"pointer("              { return TransformationParser::POINTER; }
"compound"              { return TransformationParser::COMPOUND; }
"array"                 { return TransformationParser::ARRAY; }
"r["                    { return TransformationParser::REGOF; }
"m["                    { return TransformationParser::MEMOF; }
"a["                    { return TransformationParser::ADDROF; }
"true"                  { return TransformationParser::BOOL_TRUE; }
"false"                 { return TransformationParser::BOOL_FALSE; }
{L}({L}|{D})*\(         { yytext[--yyleng] = '\0'; yylval.str = strdup(yytext); return TransformationParser::FUNC; }

{L}({L}|{D})*           { yylval.str = strdup(yytext); return TransformationParser::IDENTIFIER; }

0[xX]{H}+{IS}?          |
0{D}+{IS}?              |
{D}+{IS}?               { yylval.ival = strtol(yytext, NULL, 0); return TransformationParser::CONSTANT; }
L?'(\\.|[^\\'])+'       { return TransformationParser::CONSTANT; }

{D}+{E}{FS}?            |
{D}*"."{D}+({E})?{FS}?  |
{D}+"."{D}*({E})?{FS}?  { return TransformationParser::CONSTANT; }

L?\"(\\.|[^\\"])*\"     { yylval.str = strdup(yytext); return TransformationParser::STRING_LITERAL; }

"..."                   { return TransformationParser::ELLIPSIS; }
">>="                   { return TransformationParser::RIGHT_ASSIGN; }
"<<="                   { return TransformationParser::LEFT_ASSIGN; }
"+="                    { return TransformationParser::ADD_ASSIGN; }
"-="                    { return TransformationParser::SUB_ASSIGN; }
"*="                    { return TransformationParser::MUL_ASSIGN; }
"/="                    { return TransformationParser::DIV_ASSIGN; }
"%="                    { return TransformationParser::MOD_ASSIGN; }
"&="                    { return TransformationParser::AND_ASSIGN; }
"^="                    { return TransformationParser::XOR_ASSIGN; }
"|="                    { return TransformationParser::OR_ASSIGN; }
">>"                    { return TransformationParser::RIGHT_OP; }
"<<"                    { return TransformationParser::LEFT_OP; }
"++"                    { return TransformationParser::INC_OP; }
"--"                    { return TransformationParser::DEC_OP; }
"->"                    { return TransformationParser::PTR_OP; }
"&&"                    { return TransformationParser::AND_OP; }
"||"                    { return TransformationParser::OR_OP; }
"<="                    { return TransformationParser::LE_OP; }
">="                    { return TransformationParser::GE_OP; }
"=="                    { return TransformationParser::EQ_OP; }
"!="                    { return TransformationParser::NE_OP; }
";"                     |
"{"                     |
"}"                     |
","                     |
":"                     |
"="                     |
"("                     |
")"                     |
"["                     |
"]"                     |
"."                     |
"&"                     |
"!"                     |
"~"                     |
"-"                     |
"+"                     |
"*"                     |
"/"                     |
"%"                     |
"<"                     |
">"                     |
"^"                     |
"|"                     |
"?"                     { return yytext[0]; }

[ \t\v\n\f]+            ;
.                       ; /* ignore bad characters */

%%

void TransformationScanner::count()
{
	for (size_t i = 0; yytext[i]; ++i) {
		if (yytext[i] == '\n') {
			lineBuf.clear();
			theLine++;
			column = 0;
		} else {
			lineBuf.push_back(yytext[i]);
			if (yytext[i] == '\t')
				column += 8 - (column % 8);
			else
				column++;
		}
	}
}
