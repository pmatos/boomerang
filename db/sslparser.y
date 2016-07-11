/**
 * \file
 * \brief Defines a parser class that reads an SSL specification and returns
 *        the list of SSL instruction and table definitions.
 *
 * \authors
 * Copyright (C) 1997, Shane Sendall
 * \authors
 * Copyright (C) 1998-2001, The University of Queensland
 * \authors
 * Copyright (C) 1998, David Ung
 * \authors
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * \copyright
 * See the file "LICENSE.TERMS" for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */

%name SSLParser

%header{
	#include "types.h"
	#include "operator.h"
	#include "rtl.h"
	#include "table.h"
	#include "insnameelem.h"
	#include "statement.h"

	class SSLScanner;
%}

%union {
	Exp            *exp;
	OPER            op;
	const char     *str;
	int             num;
	double          dbl;
	Statement      *regtransfer;
	Type           *typ;

	Table          *tab;
	InsNameElem    *insel;
	std::list<std::string>   *parmlist;
	std::list<std::string>   *strlist;
	std::deque<OPER>         *oplist;
	std::deque<Exp *>        *exprlist;
	std::deque<std::string>  *namelist;
	std::list<Exp *>         *explist;
	RTL            *rtlist;
	bool            endianness;
}

%{
	#include "sslscanner.h"

	#include "gc.h"

	#include <sstream>
	#include <string>

	#include <cstdlib>
	#include <cassert>

	OPER strToTerm(const char *s);                 // Convert string to a Terminal (if possible)
	Exp *listExpToExp(std::list<Exp *> *le);       // Convert a STL list of Exp* to opList
	Exp *listStrToExp(std::list<std::string> *ls); // Convert a STL list of strings to opList
%}

%define DEBUG 1

// %define INHERIT : public gc  // This is how to force the parser class to be declared as derived from class gc

%define MEMBERS \
	public: \
		SSLParser(std::istream &in, bool trace); \
		virtual ~SSLParser(); \
		static Statement *parseExp(const char *str); /* Parse an expression or assignment from a string */ \
		/* The code for expanding tables and saving to the dictionary */ \
		void expandTables(InsNameElem *iname, std::list<std::string> *params, RTL *o_rtlist, RTLInstDict &Dict); \
		Exp *makeSuccessor(Exp *e); /* Get successor (of register expression) */ \
	\
		/* \
		 * The scanner. \
		 */ \
		SSLScanner *theScanner; \
	protected: \
	\
		/* \
		 * The file from which the SSL spec is read. \
		 */ \
		std::string sslFile; \
	\
		/* \
		 * Result for parsing an assignment. \
		 */ \
		Statement *the_asgn; \
	\
		/* \
		 * Maps SSL constants to their values. \
		 */ \
		std::map<std::string, int> ConstTable; \
	\
		/* \
		 * maps index names to instruction name-elements \
		 */ \
		std::map<std::string, InsNameElem *> indexrefmap; \
	\
		/* \
		 * Maps table names to Table's.\
		 */ \
		std::map<std::string, Table *> TableDict; \
	\
		/* \
		 * True when FLOAT keyword seen; false when INTEGER keyword seen \
		 * (in @REGISTER section) \
		 */ \
		bool bFloat;

%define CONSTRUCTOR_PARAM const std::string &sslFile, bool trace
%define CONSTRUCTOR_INIT : sslFile(sslFile), bFloat(false)
%define CONSTRUCTOR_CODE \
	std::fstream *fin = new std::fstream(sslFile.c_str(), std::ios::in); \
	theScanner = NULL; \
	if (!*fin) { \
		std::cerr << "can't open `" << sslFile << "' for reading\n"; \
		return; \
	} \
	theScanner = new SSLScanner(*fin, trace); \
	if (trace) yydebug = 1;

%define PARSE_PARAM RTLInstDict &Dict

/*==============================================================================
 * Declaration of token types, associativity and precedence
 *============================================================================*/

%token <op>  LOG_OP COND_OP BIT_OP ARITH_OP FARITH_OP
%token <op>  FPUSH FPOP NOT LNOT FNEG S_E
%token <op>  ADDR CONV_FUNC TRUNC_FUNC FABS_FUNC TRANSCEND
%token <str> NAME ASSIGNTYPE
%token <str> REG_ID DECOR
%token <str> TEMP
%token <str> NAME_CALL NAME_LOOKUP

%token       ENDIANNESS BIG LITTLE
%token       COVERS INDEX
%token       SHARES THEN LOOKUP_RDC
%token       ASSIGN TO AT REG_IDX EQUATE
%token       MEM_IDX TOK_INTEGER TOK_FLOAT FAST OPERAND
%token       FETCHEXEC CAST_OP FLAGMACRO SUCCESSOR

%token <num> NUM REG_NUM
%token <dbl> FLOATNUM       // I'd prefer type double here!

%token

%left LOG_OP
%right COND_OP
%left BIT_OP
%left ARITH_OP
%left FARITH_OP
%right NOT LNOT FCHS
%left CAST_OP
%left LOOKUP_RDC
%left S_E               // Sign extend. Note it effectively has low precedence, because being a post operator,
                        // the whole expression is already parsed, and hence is sign extended.
                        // Another reason why ! is deprecated!
%nonassoc AT

%type <exp> exp location exp_term
%type <op> bin_oper
%type <str> param
%type <regtransfer> rt assign_rt
%type <typ> assigntype
%type <num> cast
%type <tab> table_expr
%type <insel> name_contract instr_name instr_elem
%type <strlist> reg_table
%type <parmlist> list_parameter func_parameter
%type <namelist> str_term str_expr str_array name_expand
%type <explist> flag_list
%type <oplist> opstr_expr opstr_array
%type <exprlist> exprstr_expr exprstr_array
%type <explist> list_actualparameter
%type <rtlist> rt_list
%type <endianness> esize

%%

specorasgn
	: assign_rt { the_asgn = $1; }
	| exp       { the_asgn = new Assign(new Terminal(opNil), $1); }
	| specification
	;

specification
	: specification parts ';'
	| parts ';'
	;

parts
	: instr
	| FETCHEXEC rt_list { Dict.fetchExecCycle = $2; }

	/* Name := value */
	| constants

	| table_assign

	/* Optional one-line section declaring endianness */
	| endianness

	/* Optional section describing faster versions of instructions (e.g. that don't inplement the full
	 * specifications, but if they work, will be much faster) */
	| fastlist

	/* Definitions of registers (with overlaps, etc) */
	| reglist

	/* Declaration of "flag functions". These describe the detailed flag setting semantics for insructions */
	| flag_fnc

	/* Addressing modes (or instruction operands) (optional) */
	| OPERAND operandlist { Dict.fixupParams(); }
	;

operandlist
	: operandlist ',' operand
	| operand
	;

operand
	/* In the .tex documentation, this is the first, or variant kind
	 * Example: reg_or_imm := { imode, rmode }; */
	: param EQUATE '{' list_parameter '}' {
		// Note: the below copies the list of strings!
		Dict.DetParamMap[$1].params = *$4;
		Dict.DetParamMap[$1].kind = PARAM_VARIANT;
		//delete $4;
	  }

	/* In the documentation, these are the second and third kinds
	 * The third kind is described as the functional, or lambda, form
	 * In terms of DetParamMap[].kind, they are PARAM_EXP unless there
	 * actually are parameters in square brackets, in which case it is
	 * PARAM_LAMBDA
	 * Example: indexA  rs1, rs2 *i32* r[rs1] + r[rs2] */
	| param list_parameter func_parameter assigntype exp {
		ParamEntry &param = Dict.DetParamMap[$1];
		Statement *asgn = new Assign($4, new Terminal(opNil), $5);
		// Note: The below 2 copy lists of strings (to be deleted below!)
		param.params = *$2;
		param.funcParams = *$3;
		param.asgn = asgn;
		param.kind = PARAM_ASGN;

		if (param.funcParams.size() != 0)
			param.kind = PARAM_LAMBDA;
		//delete $2;
		//delete $3;
	  }
	;

func_parameter
	: '[' list_parameter ']' { $$ = $2; }
	| /* empty */            { $$ = new std::list<std::string>(); }
	;

reglist
	: TOK_INTEGER { bFloat = false; } a_reglists
	| TOK_FLOAT   { bFloat = true; }  a_reglists
	;

a_reglists
	: a_reglists ',' a_reglist
	| a_reglist
	;

a_reglist
	: REG_ID INDEX NUM {
		if (Dict.RegMap.find($1) != Dict.RegMap.end())
			yyerror("Name reglist decared twice");
		Dict.RegMap[$1] = $3;
	  }

	| REG_ID '[' NUM ']' INDEX NUM {
		if (Dict.RegMap.find($1) != Dict.RegMap.end())
			yyerror("Name reglist declared twice");
		Dict.addRegister($1, $6, $3, bFloat);
	  }

	| REG_ID '[' NUM ']' INDEX NUM COVERS REG_ID TO REG_ID {
		if (Dict.RegMap.find($1) != Dict.RegMap.end())
			yyerror("Name reglist declared twice");
		Dict.RegMap[$1] = $6;
		// Now for detailed Reg information
		if (Dict.DetRegMap.find($6) != Dict.DetRegMap.end())
			yyerror("Index used for more than one register");
		Dict.DetRegMap[$6].s_name($1);
		Dict.DetRegMap[$6].s_size($3);
		Dict.DetRegMap[$6].s_address(NULL);
		// check range is legitimate for size. 8,10
		if ((Dict.RegMap.find($8)  == Dict.RegMap.end())
		 || (Dict.RegMap.find($10) == Dict.RegMap.end()))
			yyerror("Undefined range");
		else {
			int bitsize = Dict.DetRegMap[Dict.RegMap[$10]].g_size();
			for (int i = Dict.RegMap[$8]; i != Dict.RegMap[$10]; i++) {
				if (Dict.DetRegMap.find(i) == Dict.DetRegMap.end()) {
					yyerror("Not all registers in range defined");
					break;
				}
				bitsize += Dict.DetRegMap[i].g_size();
				if (bitsize > $3) {
					yyerror("Range exceeds size of register");
					break;
				}
			}
		if (bitsize < $3)
			yyerror("Register size is exceeds registers in range");
			// copy information
		}
		Dict.DetRegMap[$6].s_mappedIndex(Dict.RegMap[$8]);
		Dict.DetRegMap[$6].s_mappedOffset(0);
		Dict.DetRegMap[$6].s_float(bFloat);
	  }

	| REG_ID '[' NUM ']' INDEX NUM SHARES REG_ID AT '[' NUM TO NUM ']' {
		if (Dict.RegMap.find($1) != Dict.RegMap.end())
			yyerror("Name reglist declared twice");
		Dict.RegMap[$1] = $6;
		// Now for detailed Reg information
		if (Dict.DetRegMap.find($6) != Dict.DetRegMap.end())
			yyerror("Index used for more than one register");
		Dict.DetRegMap[$6].s_name($1);
		Dict.DetRegMap[$6].s_size($3);
		Dict.DetRegMap[$6].s_address(NULL);
		// Do checks
		if ($3 != ($13 - $11) + 1)
			yyerror("Size does not equal range");
			if (Dict.RegMap.find($8) != Dict.RegMap.end()) {
				if ($13 >= Dict.DetRegMap[Dict.RegMap[$8]].g_size())
					yyerror("Range extends over target register");
			} else
				yyerror("Shared index not yet defined");
		Dict.DetRegMap[$6].s_mappedIndex(Dict.RegMap[$8]);
		Dict.DetRegMap[$6].s_mappedOffset($11);
		Dict.DetRegMap[$6].s_float(bFloat);
	  }

	| '[' reg_table ']' '[' NUM ']' INDEX NUM TO NUM {
		if ((int)$2->size() != ($10 - $8 + 1)) {
			std::cerr << "size of register array does not match mapping to r[" << $8 << ".." << $10 << "]\n";
			exit(1);
		} else {
			std::list<std::string>::iterator loc = $2->begin();
			for (int x = $8; x <= $10; x++, loc++) {
				if (Dict.RegMap.find(*loc) != Dict.RegMap.end())
					yyerror("Name reglist declared twice");
				Dict.addRegister(loc->c_str(), x, $5, bFloat);
			}
			//delete $2;
		}
	  }

	| '[' reg_table ']' '[' NUM ']' INDEX NUM {
		std::list<std::string>::iterator loc = $2->begin();
		for (; loc != $2->end(); loc++) {
			if (Dict.RegMap.find(*loc) != Dict.RegMap.end())
				yyerror("Name reglist declared twice");
			Dict.addRegister(loc->c_str(), $8, $5, bFloat);
		}
		//delete $2;
	  }
	;

reg_table
	: reg_table ',' REG_ID                    { $1->push_back($3); }
	| REG_ID { $$ = new std::list<std::string>; $$->push_back($1); }
	;

/* Flag definitions */
flag_fnc
	: NAME_CALL list_parameter ')' '{' rt_list '}' {
		// Note: $2 is a list of strings
		Dict.FlagFuncs[$1] = new FlagDef(listStrToExp($2), $5);
	  }
	;

constants
	: NAME EQUATE NUM {
		if (ConstTable.find($1) != ConstTable.end())
			yyerror("Constant declared twice");
		ConstTable[std::string($1)] = $3;
	  }

	| NAME EQUATE NUM ARITH_OP NUM {
		if (ConstTable.find($1) != ConstTable.end())
			yyerror("Constant declared twice");
		else if ($4 == opMinus)
			ConstTable[std::string($1)] = $3 - $5;
		else if ($4 == opPlus)
			ConstTable[std::string($1)] = $3 + $5;
		else
			yyerror("Constant expression must be NUM + NUM or NUM - NUM");
	  }
	;


table_assign
	: NAME EQUATE table_expr { TableDict[$1] = $3; }
	;

table_expr
	: str_expr     { $$ = new Table(*$1);     /*delete $1;*/ }
	| opstr_expr   { $$ = new OpTable(*$1);   /*delete $1;*/ }
	| exprstr_expr { $$ = new ExprTable(*$1); /*delete $1;*/ }
	;

str_expr
	: str_expr str_term {
		// cross-product of two str_expr's
		std::deque<std::string>::iterator i, j;
		$$ = new std::deque<std::string>;
		for (i = $1->begin(); i != $1->end(); i++)
			for (j = $2->begin(); j != $2->end(); j++)
				$$->push_back((*i) + (*j));
		//delete $1;
		//delete $2;
	  }
	| str_term { $$ = $1; }
	;

str_array
	: str_array ',' str_expr {
		// want to append $3 to $1
		// The following causes a massive warning message about mixing signed and unsigned
		$1->insert($1->end(), $3->begin(), $3->end());
		//delete $3;
		$$ = $1;
	  }
	| str_array ',' '"' '"' { $1->push_back(""); }
	| str_expr              { $$ = $1; }
	;

str_term
	: '{' str_array '}' { $$ = $2; }
	| name_expand       { $$ = $1; }
	;

name_expand
	: '\'' NAME '\'' {
		$$ = new std::deque<std::string>;
		$$->push_back("");
		$$->push_back($2);
	  }

	| '"' NAME '"' {
		$$ = new std::deque<std::string>(1, $2);
	  }

	| '$' NAME {
		std::ostringstream o;
		// expand $2 from table of names
		if (TableDict.find($2) != TableDict.end())
			if (TableDict[$2]->getType() == NAMETABLE)
				$$ = new std::deque<std::string>(TableDict[$2]->records);
			else {
				o << "Name " << $2 << " is not a NAMETABLE";
				yyerror(o.str().c_str());
			}
		else {
			o << "Could not dereference name " << $2;
			yyerror(o.str().c_str());
		}
	  }

	| NAME {
		// try and expand $1 from table of names. if fail, expand using '"' NAME '"' rule
		if (TableDict.find($1) != TableDict.end())
			if (TableDict[$1]->getType() == NAMETABLE)
				$$ = new std::deque<std::string>(TableDict[$1]->records);
			else {
				std::ostringstream o;
				o << "Name " << $1 << " is not a NAMETABLE";
				yyerror(o.str().c_str());
			}
		else {
			$$ = new std::deque<std::string>;
			$$->push_back($1);
		}
	  }
	;

bin_oper
	: BIT_OP    { $$ = $1; }
	| ARITH_OP  { $$ = $1; }
	| FARITH_OP { $$ = $1; }
	;

/* Example: OP2 := { "<<",  ">>",  ">>A" }; */
opstr_expr
	: '{' opstr_array '}' { $$ = $2; }
	;

opstr_array
	: opstr_array ',' '"' bin_oper '"'   { $$ = $1; $$->push_back($4); }
	| '"' bin_oper '"' { $$ = new std::deque<OPER>; $$->push_back($2); }
	;

/* Example: COND1_C := { "~%ZF", "%ZF", "~(%ZF | (%NF ^ %OF))", ... }; */
exprstr_expr
	: '{' exprstr_array '}' { $$ = $2; }
	;

exprstr_array
	: exprstr_array ',' '"' exp '"'  { $$ = $1; $$->push_back($4); }
	| '"' exp '"' { $$ = new std::deque<Exp *>; $$->push_back($2); }
	;

instr
	: instr_name { $1->getrefmap(indexrefmap); } list_parameter rt_list {
		// This function expands the tables and saves the expanded RTLs to the dictionary
		expandTables($1, $3, $4, Dict);
	  }
	;

instr_name
	: instr_elem { $$ = $1; }
	| instr_name DECOR {
		unsigned i;
		InsNameElem *temp;
		std::string nm = $2;

		if (nm[0] == '^')
			nm.replace(0, 1, "");

		// remove all " and _, from the decoration
		while ((i = nm.find("\"")) != nm.npos)
			nm.replace(i, 1, "");
		// replace all '.' with '_'s from the decoration
		while ((i = nm.find(".")) != nm.npos)
			nm.replace(i, 1, "_");
		while ((i = nm.find("_")) != nm.npos)
			nm.replace(i, 1, "");

		temp = new InsNameElem(nm.c_str());
		$$ = $1;
		$$->append(temp);
	  }
	;

instr_elem
	: NAME                     { $$ = new InsNameElem($1); }
	| name_contract            { $$ = $1; }
	| instr_elem name_contract { $$ = $1; $$->append($2); }
	;

name_contract
	: '\'' NAME '\'' { $$ = new InsOptionElem($2); }

	| NAME_LOOKUP NUM ']' {
		std::ostringstream o;
		if (TableDict.find($1) == TableDict.end()) {
			o << "Table " << $1 << " has not been declared";
			yyerror(o.str().c_str());
		} else if (($2 < 0) || ($2 >= (int)TableDict[$1]->records.size())) {
			o << "Can't get element " << $2 << " of table " << $1;
			yyerror(o.str().c_str());
		} else
			$$ = new InsNameElem(TableDict[$1]->records[$2].c_str());
	  }

	/* Example: ARITH[IDX]  where ARITH := { "ADD", "SUB", ... }; */
	| NAME_LOOKUP NAME ']' {
		std::ostringstream o;
		if (TableDict.find($1) == TableDict.end()) {
			o << "Table " << $1 << " has not been declared";
			yyerror(o.str().c_str());
		} else
			$$ = new InsListElem($1, TableDict[$1], $2);
	  }

	| '$' NAME_LOOKUP NUM ']' {
		std::ostringstream o;
		if (TableDict.find($2) == TableDict.end()) {
			o << "Table " << $2 << " has not been declared";
			yyerror(o.str().c_str());
		} else if (($3 < 0) || ($3 >= (int)TableDict[$2]->records.size())) {
			o << "Can't get element " << $3 << " of table " << $2;
			yyerror(o.str().c_str());
		} else
			$$ = new InsNameElem(TableDict[$2]->records[$3].c_str());
	  }

	| '$' NAME_LOOKUP NAME ']' {
		std::ostringstream o;
		if (TableDict.find($2) == TableDict.end()) {
			o << "Table " << $2 << " has not been declared";
			yyerror(o.str().c_str());
		} else
			$$ = new InsListElem($2, TableDict[$2], $3);
	  }

	| '"' NAME '"' { $$ = new InsNameElem($2); }
	;

rt_list
	: rt_list rt {
		// append any automatically generated register transfers and clear the list they were stored in.
		// Do nothing for a NOP (i.e. $2 = 0)
		if ($2 != NULL) {
			$1->appendStmt($2);
		}
		$$ = $1;
	  }
	| rt {
		$$ = new RTL(STMT_ASSIGN);
		if ($1 != NULL)
			$$->appendStmt($1);
	  }
	;

rt
	: assign_rt { $$ = $1; }

	/* Example: ADDFLAGS(r[tmp], reg_or_imm, r[rd]) */
	| NAME_CALL list_actualparameter ')' {
		std::ostringstream o;
		if (Dict.FlagFuncs.find($1) != Dict.FlagFuncs.end()) {
			// Note: SETFFLAGS assigns to the floating point flags. All others to the integer flags
			bool bFloat = strcmp($1, "SETFFLAGS") == 0;
			OPER op = bFloat ? opFflags : opFlags;
			$$ = new Assign(new Terminal(op), new Binary(opFlagCall,
			                                             new Const($1),
			                                             listExpToExp($2)));
		} else {
			o << $1 << " is not declared as a flag function";
			yyerror(o.str().c_str());
		}
	  }

	| FLAGMACRO flag_list ')' { $$ = 0; }

	/* E.g. undefineflags() (but we don't handle this yet... flags are changed, but not in a way we support) */
	| FLAGMACRO ')' { $$ = 0; }
	| '_'           { $$ = NULL; }
	;

flag_list
	: flag_list ',' REG_ID {
		// Not sure why the below is commented out (MVE)
		/*Location *pFlag = Location::regOf(Dict.RegMap[$3]);
		$1->push_back(pFlag);
		$$ = $1;*/
		$$ = 0;
	  }
	| REG_ID {
		/*std::list<Exp *> *tmp = new std::list<Exp *>;
		Unary *pFlag = new Unary(opIdRegOf, Dict.RegMap[$1]);
		tmp->push_back(pFlag);
		$$ = tmp;*/
		$$ = 0;
	  }
	;

/* Note: this list is a list of strings (other code needs this) */
list_parameter
	: list_parameter ',' param    { assert($3 != 0); $1->push_back($3); $$ = $1; }
	| param       { $$ = new std::list<std::string>; $$->push_back($1); }
	| /* empty */ { $$ = new std::list<std::string>; }
	;

param
	: NAME {
		Dict.ParamSet.insert($1);  // MVE: Likely wrong. Likely supposed to be OPERAND params only
		$$ = $1;
	  }
	;

list_actualparameter
	: list_actualparameter ',' exp           { $$->push_back($3); }
	| exp         { $$ = new std::list<Exp *>; $$->push_back($1); }
	| /* empty */ { $$ = new std::list<Exp *>; }
	;

assign_rt
	/*   Size   guard =>    lhs      :=   rhs */
	: assigntype exp THEN location EQUATE exp {
		Assign *a = new Assign($1, $4, $6);
		a->setGuard($2);
		$$ = a;
	  }
	/*   Size      lhs      :=   rhs */
	| assigntype location EQUATE exp {
		// update the size of any generated RT's
		$$ = new Assign($1, $2, $4);
	  }

	/* FPUSH and FPOP are special "transfers" with just a Terminal */
	| FPUSH { $$ = new Assign(new Terminal(opNil), new Terminal($1)); }
	| FPOP  { $$ = new Assign(new Terminal(opNil), new Terminal($1)); }

	/* Just a RHS? Is this used? Note: flag calls are handled at the rt: level */
	| assigntype exp { $$ = new Assign($1, NULL, $2); }
	;

exp_term
	: NUM                         { $$ = new Const($1); }
	| FLOATNUM                    { $$ = new Const($1); }
	| '(' exp ')'                 { $$ = $2; }
	| location                    { $$ = $1; }
	| '[' exp '?' exp ':' exp ']' { $$ = new Ternary(opTern, $2, $4, $6); }

	/* Address-of, for LEA type instructions */
	| ADDR exp ')' { $$ = new Unary($1, $2); }

	/* Conversion functions, e.g. fsize(32, 80, modrm). Args are FROMsize, TOsize, EXPression */
	| CONV_FUNC NUM ',' NUM ',' exp ')' { $$ = new Ternary($1, new Const($2), new Const($4), $6); }

	/* Truncation function: ftrunc(3.01) == 3.00 */
	| TRUNC_FUNC exp ')' { $$ = new Unary($1, $2); }

	/* fabs function: fabs(-3.01) == 3.01 */
	| FABS_FUNC exp ')' { $$ = new Unary($1, $2); }

	/* FPUSH and FPOP */
	| FPUSH { $$ = new Terminal($1); }
	| FPOP  { $$ = new Terminal($1); }

	/* Transcendental functions */
	| TRANSCEND exp ')' { $$ = new Unary($1, $2); }

	/* Example: *Use* of COND[idx] */
	| NAME_LOOKUP NAME ']' {
		std::ostringstream o;
		if (indexrefmap.find($2) == indexrefmap.end()) {
			o << "Index " << $2 << " not declared for use";
			yyerror(o.str().c_str());
		} else if (TableDict.find($1) == TableDict.end()) {
			o << "Table " << $1 << " not declared for use";
			yyerror(o.str().c_str());
		} else if (TableDict[$1]->getType() != EXPRTABLE) {
			o << "Table " << $1 << " is not an expression table but appears to be used as one";
			yyerror(o.str().c_str());
		} else if ((int)((ExprTable *)TableDict[$1])->expressions.size() < indexrefmap[$2]->ntokens()) {
			o << "Table " << $1 << " (" << ((ExprTable *)TableDict[$1])->expressions.size()
			  << ") is too small to use " << $2 << " (" << indexrefmap[$2]->ntokens() << ") as an index";
			yyerror(o.str().c_str());
		}
		// $1 is a map from string to Table*; $2 is a map from string to InsNameElem*
		$$ = new Binary(opExpTable, new Const($1), new Const($2));
	  }

	/* This is a "lambda" function-like parameter
	 * $1 is the "function" name, and $2 is a list of Exp* for the actual params.
	 * I believe only PA/RISC uses these so far. */
	| NAME_CALL list_actualparameter ')' {
		std::ostringstream o;
		if (Dict.ParamSet.find($1) != Dict.ParamSet.end()) {
			if (Dict.DetParamMap.find($1) != Dict.DetParamMap.end()) {
				ParamEntry &param = Dict.DetParamMap[$1];
				if ($2->size() != param.funcParams.size()) {
					o << $1 << " requires " << param.funcParams.size() << " parameters, but received " << $2->size();
					yyerror(o.str().c_str());
				} else {
					// Everything checks out. *phew*
					// Note: the below may not be right! (MVE)
					$$ = new Binary(opFlagDef, new Const($1), listExpToExp($2));
					//delete $2;  // Delete the list of char*s
				}
			} else {
				o << $1 << " is not defined as a OPERAND function";
				yyerror(o.str().c_str());
			}
		} else {
			o << "Unrecognized name " << $1 << " in lambda call";
			yyerror(o.str().c_str());
		}
	  }

	| SUCCESSOR exp ')' { $$ = makeSuccessor($2); }
	;

exp
	: exp S_E { $$ = new Unary($2, $1); }

	/* "%prec CAST_OP" just says that this operator has the precedence of the dummy terminal CAST_OP
	 * It's a "precedence modifier" (see "Context-Dependent Precedence" in the Bison documantation) */
	| exp cast %prec CAST_OP {
		// size casts and the opSize operator were generally deprecated, but now opSize is used to transmit
		// the size of operands that could be memOfs from the decoder to type analysis
		if ($2 == STD_SIZE)
			$$ = $1;
		else
			$$ = new Binary(opSize, new Const($2), $1);
	  }

	| NOT  exp { $$ = new Unary($1, $2); }
	| LNOT exp { $$ = new Unary($1, $2); }
	| FNEG exp { $$ = new Unary($1, $2); }

	| exp FARITH_OP exp { $$ = new Binary($2, $1, $3); }
	| exp ARITH_OP  exp { $$ = new Binary($2, $1, $3); }
	| exp BIT_OP    exp { $$ = new Binary($2, $1, $3); }
	| exp COND_OP   exp { $$ = new Binary($2, $1, $3); }
	| exp LOG_OP    exp { $$ = new Binary($2, $1, $3); }

	/* See comment above re "%prec LOOKUP_RDC"
	 * Example: OP1[IDX] where OP1 := {  "&",  "|", "^", ... }; */
	| exp NAME_LOOKUP NAME ']' exp_term %prec LOOKUP_RDC {
		std::ostringstream o;
		if (indexrefmap.find($3) == indexrefmap.end()) {
			o << "Index " << $3 << " not declared for use";
			yyerror(o.str().c_str());
		} else if (TableDict.find($2) == TableDict.end()) {
			o << "Table " << $2 << " not declared for use";
			yyerror(o.str().c_str());
		} else if (TableDict[$2]->getType() != OPTABLE) {
			o << "Table " << $2 << " is not an operator table but appears to be used as one";
			yyerror(o.str().c_str());
		} else if ((int)((OpTable *)TableDict[$2])->operators.size() < indexrefmap[$3]->ntokens()) {
			o << "Table " << $2 << " is too small to use with " << $3 << " as an index";
			yyerror(o.str().c_str());
		}
		$$ = new Ternary(opOpTable,
		                 new Const($2),
		                 new Const($3),
		                 new Binary(opList,
		                            $1,
		                            new Binary(opList,
		                                       $5,
		                                       new Terminal(opNil))));
	  }

	| exp_term { $$ = $1; }
	;

location
	/* This is for constant register numbers. Often, these are special, in the sense that the register mapping
	 * is -1. If so, the equivalent of a special register is generated, i.e. a Terminal or opMachFtr
	 * (machine specific feature) representing that register. */
	: REG_ID {
		bool isFlag = strstr($1, "flags") != 0;
		std::map<std::string, int>::const_iterator it = Dict.RegMap.find($1);
		if (it == Dict.RegMap.end() && !isFlag) {
			std::ostringstream o;
			o << "Register `" << $1 << "' is undefined";
			yyerror(o.str().c_str());
		} else if (isFlag || it->second == -1) {
			// A special register, e.g. %npc or %CF. Return a Terminal for it
			OPER op = strToTerm($1);
			if (op) {
				$$ = new Terminal(op);
			} else {
				$$ = new Unary(opMachFtr,  // Machine specific feature
				               new Const($1));
			}
		}
		else {
			// A register with a constant reg nmber, e.g. %g2.  In this case, we want to return r[const 2]
			$$ = Location::regOf(it->second);
		}
	  }

	| REG_IDX exp ']' { $$ = Location::regOf($2); }
	| REG_NUM         { $$ = Location::regOf($1); }
	| MEM_IDX exp ']' { $$ = Location::memOf($2); }

	| NAME {
		// This is a mixture of the param: PARM {} match and the value_op: NAME {} match
		Exp *s;
		std::set<std::string>::iterator it = Dict.ParamSet.find($1);
		if (it != Dict.ParamSet.end()) {
			s = new Location(opParam, new Const($1), NULL);
		} else if (ConstTable.find($1) != ConstTable.end()) {
			s = new Const(ConstTable[$1]);
		} else {
			std::ostringstream o;
			o << "`" << $1 << "' is not a constant, definition or a parameter of this instruction";
			yyerror(o.str().c_str());
			s = new Const(0);
		}
		$$ = s;
	  }

	| exp AT '[' exp ':' exp ']' { $$ = new Ternary(opAt, $1, $4, $6); }

	| TEMP { $$ = Location::tempOf(new Const($1)); }

	/* This indicates a post-instruction marker (var tick) */
	| location '\'' { $$ = new Unary(opPostVar, $1); }
	| SUCCESSOR exp ')' { $$ = makeSuccessor($2); }
	;

cast
	: '{' NUM '}' { $$ = $2; }
	;

endianness
	: ENDIANNESS esize { Dict.bigEndian = $2; }
	;

esize
	: BIG    { $$ = true; }
	| LITTLE { $$ = false; }
	;

assigntype
	: ASSIGNTYPE {
		char c = $1[1];
		if (c == '*') $$ = new SizeType(0); // MVE: should remove these
		else if (isdigit(c)) {
			int size;
			// Skip star (hence +1)
			sscanf($1 + 1, "%d", &size);
			$$ = new SizeType(size);
		} else {
			int size;
			// Skip star and letter
			sscanf($1 + 2, "%d", &size);
			if (size == 0) size = STD_SIZE;
			switch (c) {
			case 'i': $$ = new IntegerType(size, 1); break;
			case 'j': $$ = new IntegerType(size, 0); break;
			case 'u': $$ = new IntegerType(size, -1); break;
			case 'f': $$ = new FloatType(size); break;
			case 'c': $$ = new CharType; break;
			default:
				std::cerr << "Unexpected char " << c << " in assign type\n";
				$$ = new IntegerType;
			}
		}
	  }
	;

/* Section for indicating which instructions to substitute when using -f (fast but not quite as exact instruction mapping) */
fastlist
	: FAST fastentries
	;

fastentries
	: fastentries ',' fastentry
	| fastentry
	;

fastentry
	: NAME INDEX NAME { Dict.fastMap[std::string($1)] = std::string($3); }
	;

%%

/*==============================================================================
 * FUNCTION:        SSLParser::SSLParser
 * OVERVIEW:        Constructor for an existing stream.
 * PARAMETERS:      The stream, whether or not to debug
 * RETURNS:         <nothing>
 *============================================================================*/
SSLParser::SSLParser(std::istream &in, bool trace) : sslFile("input"), bFloat(false)
{
	theScanner = new SSLScanner(in, trace);
	if (trace) yydebug = 1; else yydebug = 0;
}

/*==============================================================================
 * FUNCTION:        SSLParser::parseExp
 * OVERVIEW:        Parses an assignment from a string.
 * PARAMETERS:      the string
 * RETURNS:         an Assignment or NULL.
 *============================================================================*/
Statement *SSLParser::parseExp(const char *str)
{
	std::istringstream ss(str);
	SSLParser p(ss, false);  // Second arg true for debugging
	RTLInstDict d;
	p.yyparse(d);
	return p.the_asgn;
}

/*==============================================================================
 * FUNCTION:        SSLParser::~SSLParser
 * OVERVIEW:        Destructor.
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
SSLParser::~SSLParser()
{
	std::map<std::string, Table *>::iterator loc;
	if (theScanner != NULL)
		delete theScanner;
	for (loc = TableDict.begin(); loc != TableDict.end(); loc++)
		delete loc->second;
}

/*==============================================================================
 * FUNCTION:        SSLParser::yyerror
 * OVERVIEW:        Display an error message and exit.
 * PARAMETERS:      msg - an error message
 * RETURNS:         <nothing>
 *============================================================================*/
void SSLParser::yyerror(const char* msg)
{
	std::cerr << sslFile << ":" << theScanner->theLine << ": " << msg << "\n";
}

/*==============================================================================
 * FUNCTION:        SSLParser::yylex
 * OVERVIEW:        The scanner driver than returns the next token.
 * PARAMETERS:      <none>
 * RETURNS:         the next token
 *============================================================================*/
int SSLParser::yylex()
{
	int token = theScanner->yylex(yylval);
	return token;
}

OPER strToTerm(const char *s)
{
	// s could be %pc, %afp, %agp, %CF, %ZF, %OF, %NF, %DF, %flags, %fflags
	if (s[2] == 'F') {
		if (s[1] <= 'N') {
			if (s[1] == 'C') return opCF;
			if (s[1] == 'N') return opNF;
			return opDF;
		} else {
			if (s[1] == 'O') return opOF;
			return opZF;
		}
	}
	if (s[1] == 'p') return opPC;
	if (s[1] == 'a') {
		if (s[2] == 'f') return opAFP;
		if (s[2] == 'g') return opAGP;
	} else if (s[1] == 'f') {
		if (s[2] == 'l') return opFlags;
		if (s[2] == 'f') return opFflags;
	}
	return (OPER)0;
}

/*==============================================================================
 * FUNCTION:        listExpToExp
 * OVERVIEW:        Convert a list of actual parameters in the form of a STL list of Exps into one expression
 *                    (using opList)
 * NOTE:            The expressions in the list are not cloned; they are simply copied to the new opList
 * PARAMETERS:      le: the list of expressions
 * RETURNS:         The opList Expression
 *============================================================================*/
Exp *listExpToExp(std::list<Exp *> *le)
{
	Exp *e;
	Exp **cur = &e;
	Exp *end = new Terminal(opNil);  // Terminate the chain
	for (std::list<Exp *>::iterator it = le->begin(); it != le->end(); it++) {
		*cur = new Binary(opList, *it, end);
		// cur becomes the address of the address of the second subexpression
		// In other words, cur becomes a reference to the second subexp ptr
		// Note that declaring cur as a reference doesn't work (remains a reference to e)
		cur = &(*cur)->refSubExp2();
	}
	return e;
}

/*==============================================================================
 * FUNCTION:        listStrToExp
 * OVERVIEW:        Convert a list of formal parameters in the form of a STL list of strings into one expression
 *                    (using opList)
 * PARAMETERS:      ls - the list of strings
 * RETURNS:         The opList expression
 *============================================================================*/
Exp *listStrToExp(std::list<std::string> *ls)
{
	Exp *e;
	Exp **cur = &e;
	Exp *end = new Terminal(opNil);  // Terminate the chain
	for (std::list<std::string>::iterator it = ls->begin(); it != ls->end(); it++) {
		*cur = new Binary(opList, new Location(opParam, new Const((*it).c_str()), NULL), end);
		cur = &(*cur)->refSubExp2();
	}
	*cur = new Terminal(opNil);  // Terminate the chain
	return e;
}

/*==============================================================================
 * FUNCTION:        SSLParser::expandTables
 * OVERVIEW:        Expand tables in an RTL and save to dictionary
 * NOTE:            This may generate many entries
 * PARAMETERS:      iname: Parser object representing the instruction name
 *                  params: Parser object representing the instruction params
 *                  o_rtlist: Original rtlist object (before expanding)
 *                  Dict: Ref to the dictionary that will contain the results of the parse
 * RETURNS:         <nothing>
 *============================================================================*/
static Exp *srchExpr = new Binary(opExpTable,
                                  new Terminal(opWild),
                                  new Terminal(opWild));
static Exp *srchOp = new Ternary(opOpTable,
                                 new Terminal(opWild),
                                 new Terminal(opWild),
                                 new Terminal(opWild));
void init_sslparser()
{
#ifndef NO_GARBAGE_COLLECTOR
	static Exp **gc_pointers = (Exp **)GC_MALLOC_UNCOLLECTABLE(2 * sizeof *gc_pointers);
	gc_pointers[0] = srchExpr;
	gc_pointers[1] = srchOp;
#endif
}

void SSLParser::expandTables(InsNameElem *iname, std::list<std::string> *params, RTL *o_rtlist, RTLInstDict &Dict)
{
	int i, m;
	std::string nam;
	std::ostringstream o;
	m = iname->ninstructions();
	// Expand the tables (if any) in this instruction
	for (i = 0, iname->reset(); i < m; i++, iname->increment()) {
		nam = iname->getinstruction();
		// Need to make substitutions to a copy of the RTL
		RTL *rtl = o_rtlist->clone();
		int n = rtl->getNumStmt();
		for (int j = 0; j < n; j++) {
			Statement *s = rtl->elementAt(j);
			std::list<Exp *> le;
			// Expression tables
			assert(s->getKind() == STMT_ASSIGN);
			if (((Assign *)s)->searchAll(srchExpr, le)) {
				for (std::list<Exp *>::iterator it = le.begin(); it != le.end(); it++) {
					const char *tbl = ((Const *)((Binary *)*it)->getSubExp1())->getStr();
					const char *idx = ((Const *)((Binary *)*it)->getSubExp2())->getStr();
					Exp *repl =((ExprTable *)(TableDict[tbl]))->expressions[indexrefmap[idx]->getvalue()];
					s->searchAndReplace(*it, repl);
				}
			}
			// Operator tables
			Exp *res;
			while (s->search(srchOp, res)) {
				Ternary *t;
				if (res->getOper() == opTypedExp)
					t = (Ternary *)res->getSubExp1();
				else
					t = (Ternary *)res;
				assert(t->getOper() == opOpTable);
				// The ternary opOpTable has a table and index name as strings, then a list of 2 expressions
				// (and we want to replace it with e1 OP e2)
				const char *tbl = ((Const *)t->getSubExp1())->getStr();
				const char *idx = ((Const *)t->getSubExp2())->getStr();
				// The expressions to operate on are in the list
				Binary *b = (Binary *)t->getSubExp3();
				assert(b->getOper() == opList);
				Exp *e1 = b->getSubExp1();
				Exp *e2 = b->getSubExp2();  // This should be an opList too
				assert(b->getOper() == opList);
				e2 = ((Binary *)e2)->getSubExp1();
				OPER ops = ((OpTable *)(TableDict[tbl]))->operators[indexrefmap[idx]->getvalue()];
				Exp *repl = new Binary(ops, e1->clone(), e2->clone());  // FIXME!
				s->searchAndReplace(res, repl);
			}
		}

		if (Dict.appendToDict(nam, *params, *rtl) != 0) {
			o << "Pattern " << iname->getinspattern() << " conflicts with an earlier declaration of " << nam;
			yyerror(o.str().c_str());
		}
	}
	indexrefmap.erase(indexrefmap.begin(), indexrefmap.end());
}

/*==============================================================================
 * FUNCTION:        SSLParser::makeSuccessor
 * OVERVIEW:        Make the successor of the given expression, e.g. given r[2], return succ(r[2])
 *                    (using opSuccessor)
 *                  We can't do the successor operation here, because the parameters are not yet instantiated
 *                    (still of the form param(rd)). Actual successor done in Exp::fixSuccessor()
 * NOTE:            The given expression should be of the form  r[const]
 * NOTE:            The parameter expresion is copied (not cloned) in the result
 * PARAMETERS:      The expression to find the successor of
 * RETURNS:         The modified expression
 *============================================================================*/
Exp *SSLParser::makeSuccessor(Exp *e)
{
	return new Unary(opSuccessor, e);
}
