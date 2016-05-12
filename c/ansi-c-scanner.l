/**
 * \file
 * \brief Scanner for ANSI C.
 */

%name AnsiCScanner

%header{
	#include "ansi-c-parser.h"
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
		bool traceLines;    /* echo each lines as it is scanned */ \
		int column; \
	private: \
		void count(); \
		int check_type();

%define CONSTRUCTOR_PARAM std::istream &ins, bool trace
%define CONSTRUCTOR_INIT : theLine(1), traceLines(trace), column(0)
%define CONSTRUCTOR_CODE yyin = &ins;

%define LEX_PARAM YY_AnsiCParser_STYPE &yylval

D                       [0-9]
/* Allow ? and @ in identifiers, so we can use mangled names */
L                       [a-zA-Z_?@]
H                       [a-fA-F0-9]
E                       [Ee][+-]?{D}+
FS                      (f|F|l|L)
IS                      (u|U|l|L)*

%x COMMENT

%%

^#include               { return AnsiCParser::PREINCLUDE; }
^#define                { return AnsiCParser::PREDEFINE; }
^#if                    { return AnsiCParser::PREIF; }
^#ifdef                 { return AnsiCParser::PREIFDEF; }
^#endif                 { return AnsiCParser::PREENDIF; }
^#line                  { return AnsiCParser::PRELINE; }

"//".*                  ;
"/*"                    { BEGIN(COMMENT); }
<COMMENT>"*/"           { BEGIN(INITIAL); }
<COMMENT>[^*\n]+        ;
<COMMENT>"*"            ;
<COMMENT>\n             ;

"auto"                  { return AnsiCParser::AUTO; }
"break"                 { return AnsiCParser::BREAK; }
"case"                  { return AnsiCParser::CASE; }
"char"                  { return AnsiCParser::CHAR; }
"const"                 { return AnsiCParser::CONST; }
"continue"              { return AnsiCParser::CONTINUE; }
"default"               { return AnsiCParser::DEFAULT; }
"do"                    { return AnsiCParser::DO; }
"double"                { return AnsiCParser::DOUBLE; }
"else"                  { return AnsiCParser::ELSE; }
"enum"                  { return AnsiCParser::ENUM; }
"extern"                { return AnsiCParser::EXTERN; }
"float"                 { return AnsiCParser::FLOAT; }
"for"                   { return AnsiCParser::FOR; }
"goto"                  { return AnsiCParser::GOTO; }
"if"                    { return AnsiCParser::IF; }
"int"                   { return AnsiCParser::INT; }
"long"                  { return AnsiCParser::LONG; }
"register"              { return AnsiCParser::REGISTER; }
"return"                { return AnsiCParser::RETURN; }
"short"                 { return AnsiCParser::SHORT; }
"signed"                { return AnsiCParser::SIGNED; }
"sizeof"                { return AnsiCParser::SIZEOF; }
"__nodecode"            { return AnsiCParser::NODECODE; }
"__incomplete"          { return AnsiCParser::INCOMPLETE; }
"__cdecl"               { return AnsiCParser::CDECL; }
"__pascal"              { return AnsiCParser::PASCAL; }
"__stdcall"             { return AnsiCParser::PASCAL; }
"__custom"              { return AnsiCParser::CUSTOM; }
"__thiscall"            { return AnsiCParser::THISCALL; }
"PREFER"                { return AnsiCParser::PREFER; }
"__withstack("          { return AnsiCParser::WITHSTACK; }
"__symref"              { return AnsiCParser::SYMBOLREF; }
"static"                { return AnsiCParser::STATIC; }
"struct"                { return AnsiCParser::STRUCT; }
"switch"                { return AnsiCParser::SWITCH; }
"typedef"               { return AnsiCParser::TYPEDEF; }
"union"                 { return AnsiCParser::UNION; }
"unsigned"              { return AnsiCParser::UNSIGNED; }
"void"                  { return AnsiCParser::VOID; }
"volatile"              { return AnsiCParser::VOLATILE; }
"while"                 { return AnsiCParser::WHILE; }
"r["                    { return AnsiCParser::REGOF; }
"m["                    { return AnsiCParser::MEMOF; }
"@max("                 { return AnsiCParser::MAXBOUND; }

{L}({L}|{D})*           { yylval.str = strdup(yytext); return check_type(); }

0[xX]{H}+{IS}?          |
0{D}+{IS}?              |
{D}+{IS}?               { yylval.ival = strtol(yytext, NULL, 0); return AnsiCParser::CONSTANT; }
L?'(\\.|[^\\'])+'       { return AnsiCParser::CONSTANT; }

{D}+{E}{FS}?            |
{D}*"."{D}+({E})?{FS}?  |
{D}+"."{D}*({E})?{FS}?  { return AnsiCParser::CONSTANT; }

L?\"(\\.|[^\\"])*\"     { yylval.str = strdup(yytext); return AnsiCParser::STRING_LITERAL; }

"..."                   { return AnsiCParser::ELLIPSIS; }
">>="                   { return AnsiCParser::RIGHT_ASSIGN; }
"<<="                   { return AnsiCParser::LEFT_ASSIGN; }
"+="                    { return AnsiCParser::ADD_ASSIGN; }
"-="                    { return AnsiCParser::SUB_ASSIGN; }
"*="                    { return AnsiCParser::MUL_ASSIGN; }
"/="                    { return AnsiCParser::DIV_ASSIGN; }
"%="                    { return AnsiCParser::MOD_ASSIGN; }
"&="                    { return AnsiCParser::AND_ASSIGN; }
"^="                    { return AnsiCParser::XOR_ASSIGN; }
"|="                    { return AnsiCParser::OR_ASSIGN; }
">>"                    { return AnsiCParser::RIGHT_OP; }
"<<"                    { return AnsiCParser::LEFT_OP; }
"++"                    { return AnsiCParser::INC_OP; }
"--"                    { return AnsiCParser::DEC_OP; }
"->"                    { return AnsiCParser::PTR_OP; }
"&&"                    { return AnsiCParser::AND_OP; }
"||"                    { return AnsiCParser::OR_OP; }
"<="                    { return AnsiCParser::LE_OP; }
">="                    { return AnsiCParser::GE_OP; }
"=="                    { return AnsiCParser::EQ_OP; }
"!="                    { return AnsiCParser::NE_OP; }
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

void AnsiCScanner::count()
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

int AnsiCScanner::check_type()
{
#if 0 // pseudo code --- this is what it should check
	if (yytext == type_name)
		return AnsiCParser::TYPE_NAME;
	return AnsiCParser::IDENTIFIER;
#else // it actually will only return IDENTIFIER
	return AnsiCParser::IDENTIFIER;
#endif
}
