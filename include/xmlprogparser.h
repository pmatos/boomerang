/*
 * Copyright (C) 2004, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*=============================================================================
 * FILE:        XMLProgParser.h
 * OVERVIEW:    parses persisted XML output and creates a new prog.
 *============================================================================*/

#ifndef XMLPROGPARSER_H
#define XMLPROGPARSER_H

class Global;
class Cluster;
class Prog;
class Proc;
class UserProc;
class LibProc;
class Signature;
class StatementSet;
class Cluster;
class Context;
class Type;
class Exp;
class Cfg;
class BasicBlock;
class RTL;

#define MAX_STACK 1024

typedef struct {
	const char *tag;
	void (XMLProgParser::*start_proc)(const char **);
	void (XMLProgParser::*end_proc)(Context *c, int e);
} _tag;

class XMLProgParser {
public:
	XMLProgParser() { }
	Prog *parse(const char *filename);
protected:
	void parseFile(const char *filename);
	void parseChildren(Cluster *c);

public:
	void handleElementStart(const char *el, const char **attr);
	void handleElementEnd(const char *el);
protected:
	static _tag tags[];

	static const char *getAttr(const char **attr, const char *name);
	static int operFromString(const char *s);

	std::list<Context *> stack;
	std::map<int, void *> idToX;
	int phase;

	void addId(const char **attr, void *x);
	void *findId(const char *id);

#define TAGD(x) \
	void start_##x(const char **attr); \
	void addToContext_##x(Context *c, int e);

	TAGD(prog)
	TAGD(procs)
	TAGD(global)
	TAGD(cluster)
	TAGD(libproc)
	TAGD(userproc)
	TAGD(local)
	TAGD(symbol)
	TAGD(secondexp)
	TAGD(proven_true)
	TAGD(callee)
	TAGD(caller)
	TAGD(defines)
	TAGD(signature)
	TAGD(param)
	TAGD(return)
	TAGD(rettype)
	TAGD(prefreturn)
	TAGD(prefparam)
	TAGD(cfg)
	TAGD(bb)
	TAGD(inedge)
	TAGD(outedge)
	TAGD(livein)
	TAGD(order)
	TAGD(revorder)
	TAGD(rtl)
	TAGD(stmt)
	TAGD(assign)
	TAGD(assignment)
	TAGD(phiassign)
	TAGD(lhs)
	TAGD(rhs)
	TAGD(callstmt)
	TAGD(dest)
	TAGD(argument)
	TAGD(returnexp)
	TAGD(returntype)
	TAGD(returnstmt)
	TAGD(returns)
	TAGD(modifieds)
	TAGD(gotostmt)
	TAGD(branchstmt)
	TAGD(cond)
	TAGD(casestmt)
	TAGD(boolasgn)
	TAGD(type)
	TAGD(exp)
	TAGD(voidtype)
	TAGD(integertype)
	TAGD(pointertype)
	TAGD(chartype)
	TAGD(namedtype)
	TAGD(arraytype)
	TAGD(basetype)
	TAGD(sizetype)
	TAGD(location)
	TAGD(unary)
	TAGD(binary)
	TAGD(ternary)
	TAGD(const)
	TAGD(terminal)
	TAGD(typedexp)
	TAGD(refexp)
	TAGD(def)
	TAGD(subexp1)
	TAGD(subexp2)
	TAGD(subexp3)

public:
	static void persistToXML(Prog *prog);
protected:
	static void persistToXML(std::ostream &out, Global *g);
	static void persistToXML(std::ostream &out, Cluster *c);
	static void persistToXML(std::ostream &out, Proc *proc);
	static void persistToXML(std::ostream &out, LibProc *proc);
	static void persistToXML(std::ostream &out, UserProc *proc);
	static void persistToXML(std::ostream &out, Signature *sig);
	static void persistToXML(std::ostream &out, Type *ty);
	static void persistToXML(std::ostream &out, Exp *e);
	static void persistToXML(std::ostream &out, Cfg *cfg);
	static void persistToXML(std::ostream &out, BasicBlock *bb);
	static void persistToXML(std::ostream &out, RTL *rtl);
	static void persistToXML(std::ostream &out, Statement *stmt);
};

#endif
