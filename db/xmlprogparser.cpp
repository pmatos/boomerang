/*
 * Copyright (C) 2004, Trent Waddington
 * Copyright (C) 2016, Kyle Guinn
 */
/*==============================================================================
 * FILE:       xmlprogparser.cpp
 * OVERVIEW:   Implementation of the XMLProgParser and related classes.
 *============================================================================*/

#include "type.h"
#include "cluster.h"
#include "prog.h"
#include "proc.h"
#include "rtl.h"
#include "statement.h"
#include "sigenum.h"
#include "signature.h"
#include "xmlprogparser.h"
#include "boomerang.h"
#include "log.h"
#include "frontend.h"

extern "C" {
#include "expat.h"
}

#include <cstdio>
#include <cstring>

typedef enum {
	e_prog, e_procs, e_global, e_cluster, e_libproc, e_userproc, e_local, e_symbol, e_secondexp,
	e_proven_true, e_callee, e_caller, e_defines,
	e_signature, e_param, e_return, e_rettype, e_prefreturn, e_prefparam,
	e_cfg, e_bb, e_inedge, e_outedge, e_livein, e_order, e_revorder,
	e_rtl, e_stmt, e_assign, e_assignment, e_phiassign, e_lhs, e_rhs,
	e_callstmt, e_dest, e_argument, e_returnexp, e_returntype,
	e_returnstmt, e_returns, e_modifieds,
	e_gotostmt, e_branchstmt, e_cond,
	e_casestmt,
	e_boolasgn,
	e_type, e_exp,
	e_voidtype, e_integertype, e_pointertype, e_chartype, e_namedtype, e_arraytype, e_basetype, e_sizetype,
	e_location, e_unary, e_binary, e_ternary, e_const, e_terminal, e_typedexp, e_refexp, e_def,
	e_subexp1, e_subexp2, e_subexp3, e_unknown = -1
} xmlElement;

#define TAG(x) { #x, &XMLProgParser::start_##x, &XMLProgParser::addChildTo_##x }

_tag XMLProgParser::tags[] = {
	TAG(prog),
	TAG(procs),
	TAG(global),
	TAG(cluster),
	TAG(libproc),
	TAG(userproc),
	TAG(local),
	TAG(symbol),
	TAG(secondexp),
	TAG(proven_true),
	TAG(callee),
	TAG(caller),
	TAG(defines),
	TAG(signature),
	TAG(param),
	TAG(return),
	TAG(rettype),
	TAG(prefreturn),
	TAG(prefparam),
	TAG(cfg),
	TAG(bb),
	TAG(inedge),
	TAG(outedge),
	TAG(livein),
	TAG(order),
	TAG(revorder),
	TAG(rtl),
	TAG(stmt),
	TAG(assign),
	TAG(assignment),
	TAG(phiassign),
	TAG(lhs),
	TAG(rhs),
	TAG(callstmt),
	TAG(dest),
	TAG(argument),
	TAG(returnexp),
	TAG(returntype),
	TAG(returnstmt),
	TAG(returns),
	TAG(modifieds),
	TAG(gotostmt),
	TAG(branchstmt),
	TAG(cond),
	TAG(casestmt),
	TAG(boolasgn),
	TAG(type),
	TAG(exp),
	TAG(voidtype),
	TAG(integertype),
	TAG(pointertype),
	TAG(chartype),
	TAG(namedtype),
	TAG(arraytype),
	TAG(basetype),
	TAG(sizetype),
	TAG(location),
	TAG(unary),
	TAG(binary),
	TAG(ternary),
	TAG(const),
	TAG(terminal),
	TAG(typedexp),
	TAG(refexp),
	TAG(def),
	TAG(subexp1),
	TAG(subexp2),
	TAG(subexp3),
	{ 0, 0, 0 }
};

class Context {
public:
	int tag;
	int n;
	std::string str;
	Prog *prog;
	Global *global;
	Cluster *cluster;
	Proc *proc;
	Signature *signature;
	Cfg *cfg;
	BasicBlock *bb;
	RTL *rtl;
	Statement *stmt;
	Parameter *param;
	// ImplicitParameter *implicitParam;
	Return *ret;
	Type *type;
	Exp *exp, *symbol;
	std::list<Proc *> procs;

	Context(int tag) : tag(tag), prog(NULL), proc(NULL), signature(NULL), cfg(NULL), bb(NULL), rtl(NULL), stmt(NULL), param(NULL), /*implicitParam(NULL),*/ ret(NULL), type(NULL), exp(NULL) { }
};

static void XMLCALL
start(void *data, const char *el, const char **attr)
{
	((XMLProgParser *)data)->handleElementStart(el, attr);
}

static void XMLCALL
end(void *data, const char *el)
{
	((XMLProgParser *)data)->handleElementEnd(el);
}

static void XMLCALL
text(void *data, const char *s, int len)
{
	int mylen;
	char buf[1024];
	if (len == 1 && *s == '\n')
		return;
	mylen = len < 1024 ? len : 1023;
	memcpy(buf, s, mylen);
	buf[mylen] = 0;
	printf("error: text in document %i bytes (%s)\n", len, buf);
}

const char *XMLProgParser::getAttr(const char **attr, const char *name)
{
	for (int i = 0; attr[i]; i += 2)
		if (!strcmp(attr[i], name))
			return attr[i + 1];
	return NULL;
}

void XMLProgParser::handleElementStart(const char *el, const char **attr)
{
	for (int i = 0; tags[i].tag; i++)
		if (!strcmp(el, tags[i].tag)) {
			//std::cerr << "got tag: " << el << "\n";
			stack.push(new Context(i));
			(this->*tags[i].start)(stack.top(), attr);
			return;
		}
	std::cerr << "got unknown tag: " << el << "\n";
	stack.push(new Context(e_unknown));
}

void XMLProgParser::handleElementEnd(const char *el)
{
	//std::cerr << "end tag: " << el << " tos: " << stack.top()->tag << "\n";
	if (stack.size() >= 2) {
		Context *child = stack.top();
		stack.pop();
		Context *node = stack.top();
		if (node->tag != e_unknown) {
			//std::cerr << " second: " << node->tag << "\n";
			(this->*tags[node->tag].addChildTo)(node, child);
		}
	}
}

void XMLProgParser::addChildStub(Context *node, const Context *child) const
{
	if (child->tag == e_unknown)
		std::cerr << "unknown tag";
	else
		std::cerr << "need to handle tag " << tags[child->tag].tag;
	std::cerr << " in context " << tags[node->tag].tag << "\n";
}

Prog *XMLProgParser::parse(const char *filename)
{
	FILE *f = fopen(filename, "r");
	if (f == NULL)
		return NULL;
	fclose(f);

	while (!stack.empty())
		stack.pop();
	Prog *prog = NULL;
	for (phase = 0; phase < 2; phase++) {
		parseFile(filename);
		if (stack.top()->prog) {
			prog = stack.top()->prog;
			parseChildren(prog->getRootCluster());
		}
	}
	if (prog == NULL)
		return NULL;
	//FrontEnd *pFE = FrontEnd::Load(prog->getPath(), prog);  // Path is usually empty!?
	FrontEnd *pFE = FrontEnd::Load(prog->getPathAndName(), prog);
	prog->setFrontEnd(pFE);
	return prog;
}

void XMLProgParser::parseFile(const char *filename)
{
	FILE *f = fopen(filename, "r");
	if (f == NULL)
		return;
	XML_Parser p = XML_ParserCreate(NULL);
	if (!p) {
		fprintf(stderr, "Couldn't allocate memory for parser\n");
		return;
	}

	XML_SetUserData(p, this);
	XML_SetElementHandler(p, start, end);
	XML_SetCharacterDataHandler(p, text);

	char Buff[8192];

	for (;;) {
		int done;
		int len;

		len = fread(Buff, 1, sizeof(Buff), f);
		if (ferror(f)) {
			fprintf(stderr, "Read error\n");
			fclose(f);
			return;
		}
		done = feof(f);

		if (XML_Parse(p, Buff, len, done) == XML_STATUS_ERROR) {
			if (XML_GetErrorCode(p) != XML_ERROR_NO_ELEMENTS)
				fprintf(stderr, "Parse error at line %d of file %s:\n%s\n", XML_GetCurrentLineNumber(p), filename, XML_ErrorString(XML_GetErrorCode(p)));
			fclose(f);
			return;
		}

		if (done)
			break;
	}
	fclose(f);
}

void XMLProgParser::parseChildren(Cluster *c)
{
	std::string path = c->makeDirs();
	for (unsigned i = 0; i < c->children.size(); i++) {
		std::string d = path + "/" + c->children[i]->getName() + ".xml";
		parseFile(d.c_str());
		parseChildren(c->children[i]);
	}
}

extern char *operStrings[];

int XMLProgParser::operFromString(const char *s)
{
	for (int i = 0; i < opNumOf; i++)
		if (!strcmp(s, operStrings[i]))
			return i;
	return -1;
}

void XMLProgParser::addId(const char **attr, void *x)
{
	const char *val = getAttr(attr, "id");
	if (val) {
		//std::cerr << "map id " << val << " to " << std::hex << (int)x << std::dec << "\n";
		idToX[atoi(val)] = x;
	}
}

void *XMLProgParser::findId(const char *id) const
{
	if (id == NULL)
		return NULL;
	int n = atoi(id);
	if (n == 0)
		return NULL;
	std::map<int, void *>::const_iterator it = idToX.find(n);
	if (it == idToX.end()) {
		std::cerr << "findId could not find \"" << id << "\"\n";
		assert(false);
		return NULL;
	}
	return it->second;
}


void XMLProgParser::start_prog(Context *node, const char **attr)
{
	if (phase == 1) {
		return;
	}
	node->prog = new Prog();
	addId(attr, node->prog);
	const char *name = getAttr(attr, "name");
	if (name)
		node->prog->setName(name);
	name = getAttr(attr, "path");
	if (name)
		node->prog->m_path = name;
	const char *iNumberedProc = getAttr(attr, "iNumberedProc");
	node->prog->m_iNumberedProc = atoi(iNumberedProc);
}

void XMLProgParser::addChildTo_prog(Context *node, const Context *child) const
{
	if (phase == 1) {
		switch (child->tag) {
		case e_libproc:
		case e_userproc:
			Boomerang::get()->alert_load(child->proc);
			break;
		}
		return;
	}
	switch (child->tag) {
	case e_libproc:
		child->proc->setProg(node->prog);
		node->prog->m_procs.push_back(child->proc);
		node->prog->m_procLabels[child->proc->getNativeAddress()] = child->proc;
		break;
	case e_userproc:
		child->proc->setProg(node->prog);
		node->prog->m_procs.push_back(child->proc);
		node->prog->m_procLabels[child->proc->getNativeAddress()] = child->proc;
		break;
	case e_procs:
		for (std::list<Proc *>::const_iterator it = child->procs.begin(); it != child->procs.end(); it++) {
			node->prog->m_procs.push_back(*it);
			node->prog->m_procLabels[(*it)->getNativeAddress()] = *it;
			Boomerang::get()->alert_load(*it);
		}
		break;
	case e_cluster:
		node->prog->m_rootCluster = child->cluster;
		break;
	case e_global:
		node->prog->globals.insert(child->global);
		break;
	default:
		addChildStub(node, child);
		break;
	}
}

void XMLProgParser::start_procs(Context *node, const char **attr)
{
	if (phase == 1) {
		return;
	}
	node->procs.clear();
}

void XMLProgParser::addChildTo_procs(Context *node, const Context *child) const
{
	if (phase == 1) {
		return;
	}
	switch (child->tag) {
	case e_libproc:
		node->procs.push_back(child->proc);
		break;
	case e_userproc:
		node->procs.push_back(child->proc);
		break;
	default:
		addChildStub(node, child);
		break;
	}
}

void XMLProgParser::start_global(Context *node, const char **attr)
{
	if (phase == 1) {
		return;
	}
	node->global = new Global();
	addId(attr, node->global);
	const char *name = getAttr(attr, "name");
	if (name)
		node->global->nam = name;
	const char *uaddr = getAttr(attr, "uaddr");
	if (uaddr)
		node->global->uaddr = atoi(uaddr);
}

void XMLProgParser::addChildTo_global(Context *node, const Context *child) const
{
	if (phase == 1) {
		return;
	}
	switch (child->tag) {
	case e_type:
		node->global->type = child->type;
		break;
	default:
		addChildStub(node, child);
		break;
	}
}

void XMLProgParser::start_cluster(Context *node, const char **attr)
{
	if (phase == 1) {
		node->cluster = (Cluster *)findId(getAttr(attr, "id"));
		return;
	}
	node->cluster = new Cluster();
	addId(attr, node->cluster);
	const char *name = getAttr(attr, "name");
	if (name)
		node->cluster->setName(name);
}

void XMLProgParser::addChildTo_cluster(Context *node, const Context *child) const
{
	if (phase == 1) {
		return;
	}
	switch (child->tag) {
	case e_cluster:
		node->cluster->addChild(child->cluster);
		break;
	default:
		addChildStub(node, child);
		break;
	}
}

void XMLProgParser::start_libproc(Context *node, const char **attr)
{
	if (phase == 1) {
		node->proc = (Proc *)findId(getAttr(attr, "id"));
		Proc *p = (Proc *)findId(getAttr(attr, "firstCaller"));
		if (p)
			node->proc->m_firstCaller = p;
		Cluster *c = (Cluster *)findId(getAttr(attr, "cluster"));
		if (c)
			node->proc->cluster = c;
		return;
	}
	node->proc = new LibProc();
	addId(attr, node->proc);
	const char *address = getAttr(attr, "address");
	if (address)
		node->proc->address = atoi(address);
	address = getAttr(attr, "firstCallerAddress");
	if (address)
		node->proc->m_firstCallerAddr = atoi(address);
}

void XMLProgParser::addChildTo_libproc(Context *node, const Context *child) const
{
	if (phase == 1) {
		switch (child->tag) {
		case e_caller:
			CallStatement *call = dynamic_cast<CallStatement *>(child->stmt);
			assert(call);
			node->proc->addCaller(call);
			break;
		}
		return;
	}
	switch (child->tag) {
	case e_signature:
		node->proc->setSignature(child->signature);
		break;
	case e_proven_true:
		node->proc->setProvenTrue(child->exp);
		break;
	case e_caller:
		break;
	default:
		addChildStub(node, child);
		break;
	}
}

void XMLProgParser::start_userproc(Context *node, const char **attr)
{
	if (phase == 1) {
		node->proc = (Proc *)findId(getAttr(attr, "id"));
		UserProc *u = dynamic_cast<UserProc *>(node->proc);
		assert(u);
		Proc *p = (Proc *)findId(getAttr(attr, "firstCaller"));
		if (p)
			u->m_firstCaller = p;
		Cluster *c = (Cluster *)findId(getAttr(attr, "cluster"));
		if (c)
			u->cluster = c;
		ReturnStatement *r = (ReturnStatement *)findId(getAttr(attr, "retstmt"));
		if (r)
			u->theReturnStatement = r;
		return;
	}
	UserProc *proc = new UserProc();
	node->proc = proc;
	addId(attr, proc);

	const char *address = getAttr(attr, "address");
	if (address)
		proc->address = atoi(address);
	address = getAttr(attr, "status");
	if (address)
		proc->status = (ProcStatus)atoi(address);
	address = getAttr(attr, "firstCallerAddress");
	if (address)
		proc->m_firstCallerAddr = atoi(address);
}

void XMLProgParser::addChildTo_userproc(Context *node, const Context *child) const
{
	UserProc *userproc = dynamic_cast<UserProc *>(node->proc);
	assert(userproc);
	if (phase == 1) {
		switch (child->tag) {
		case e_caller:
			{
				CallStatement *call = dynamic_cast<CallStatement *>(child->stmt);
				assert(call);
				node->proc->addCaller(call);
			}
			break;
		case e_callee:
			userproc->addCallee(child->proc);
			break;
		}
		return;
	}
	switch (child->tag) {
	case e_signature:
		node->proc->setSignature(child->signature);
		break;
	case e_proven_true:
		node->proc->setProvenTrue(child->exp);
		break;
	case e_caller:
		break;
	case e_callee:
		break;
	case e_cfg:
		userproc->setCFG(child->cfg);
		break;
	case e_local:
		userproc->locals[child->str.c_str()] = child->type;
		break;
	case e_symbol:
		userproc->mapSymbolTo(child->exp, child->symbol);
		break;
	default:
		addChildStub(node, child);
		break;
	}
}

void XMLProgParser::start_local(Context *node, const char **attr)
{
	if (phase == 1) {
		return;
	}
	node->str = getAttr(attr, "name");
}

void XMLProgParser::addChildTo_local(Context *node, const Context *child) const
{
	if (phase == 1) {
		return;
	}
	switch (child->tag) {
	case e_type:
		node->type = child->type;
		break;
	default:
		addChildStub(node, child);
		break;
	}
}

void XMLProgParser::start_symbol(Context *node, const char **attr)
{
	if (phase == 1) {
		return;
	}
}

void XMLProgParser::addChildTo_symbol(Context *node, const Context *child) const
{
	if (phase == 1) {
		return;
	}
	switch (child->tag) {
	case e_exp:
		node->exp = child->exp;
		break;
	case e_secondexp:
		node->symbol = child->exp;
		break;
	default:
		addChildStub(node, child);
		break;
	}
}

void XMLProgParser::start_secondexp(Context *node, const char **attr)
{
}

void XMLProgParser::addChildTo_secondexp(Context *node, const Context *child) const
{
	node->exp = child->exp;
}

void XMLProgParser::start_proven_true(Context *node, const char **attr)
{
}

void XMLProgParser::addChildTo_proven_true(Context *node, const Context *child) const
{
	node->exp = child->exp;
}

void XMLProgParser::start_callee(Context *node, const char **attr)
{
	if (phase == 1) {
		node->proc = (Proc *)findId(getAttr(attr, "proc"));
	}
}

void XMLProgParser::addChildTo_callee(Context *node, const Context *child) const
{
}

void XMLProgParser::start_caller(Context *node, const char **attr)
{
	if (phase == 1) {
		node->stmt = (Statement *)findId(getAttr(attr, "call"));
	}
}

void XMLProgParser::addChildTo_caller(Context *node, const Context *child) const
{
}

void XMLProgParser::start_defines(Context *node, const char **attr)
{
}

void XMLProgParser::addChildTo_defines(Context *node, const Context *child) const
{
	node->exp = child->exp;
}

void XMLProgParser::start_signature(Context *node, const char **attr)
{
	if (phase == 1) {
		node->signature = (Signature *)findId(getAttr(attr, "id"));
		return;
	}
	const char *plat = getAttr(attr, "platform");
	const char *convention = getAttr(attr, "convention");
	const char *name = getAttr(attr, "name");

	Signature *sig;
	if (plat && convention) {
		platform p;
		callconv c;
		if (!strcmp(plat, "pentium"))
			p = PLAT_PENTIUM;
		else if (!strcmp(plat, "sparc"))
			p = PLAT_SPARC;
		else if (!strcmp(plat, "ppc"))
			p = PLAT_PPC;
		else if (!strcmp(plat, "st20"))
			p = PLAT_ST20;
		else {
			std::cerr << "unknown platform: " << plat << "\n";
			assert(false);
			p = PLAT_PENTIUM;
		}
		if (!strcmp(convention, "stdc"))
			c = CONV_C;
		else if (!strcmp(convention, "pascal"))
			c = CONV_PASCAL;
		else if (!strcmp(convention, "thiscall"))
			c = CONV_THISCALL;
		else {
			std::cerr << "unknown convention: " << convention << "\n";
			assert(false);
			c = CONV_C;
		}
		sig = Signature::instantiate(p, c, name);
	} else
		sig = new Signature(name);
	sig->params.clear();
	// sig->implicitParams.clear();
	sig->returns.clear();
	node->signature = sig;
	addId(attr, sig);
	const char *n = getAttr(attr, "ellipsis");
	if (n)
		sig->ellipsis = atoi(n) > 0;
	n = getAttr(attr, "preferedName");
	if (n)
		sig->preferedName = n;
}

void XMLProgParser::addChildTo_signature(Context *node, const Context *child) const
{
	if (phase == 1) {
		return;
	}
	switch (child->tag) {
	case e_param:
		node->signature->appendParameter(child->param);
		break;
	case e_return:
		node->signature->appendReturn(child->ret);
		break;
	case e_rettype:
		node->signature->setRetType(child->type);
		break;
	case e_prefparam:
		node->signature->preferedParams.push_back(child->n);
		break;
	case e_prefreturn:
		node->signature->preferedReturn = child->type;
		break;
	default:
		addChildStub(node, child);
		break;
	}
}

void XMLProgParser::start_param(Context *node, const char **attr)
{
	if (phase == 1) {
		node->param = (Parameter *)findId(getAttr(attr, "id"));
		return;
	}
	node->param = new Parameter();
	addId(attr, node->param);
	const char *n = getAttr(attr, "name");
	if (n)
		node->param->setName(n);
}

void XMLProgParser::addChildTo_param(Context *node, const Context *child) const
{
	if (phase == 1) {
		return;
	}
	switch (child->tag) {
	case e_type:
		node->param->setType(child->type);
		break;
	case e_exp:
		node->param->setExp(child->exp);
		break;
	default:
		addChildStub(node, child);
		break;
	}
}

void XMLProgParser::start_return(Context *node, const char **attr)
{
	if (phase == 1) {
		node->ret = (Return *)findId(getAttr(attr, "id"));
		return;
	}
	node->ret = new Return();
	addId(attr, node->ret);
}

void XMLProgParser::addChildTo_return(Context *node, const Context *child) const
{
	if (phase == 1) {
		return;
	}
	switch (child->tag) {
	case e_type:
		node->ret->type = child->type;
		break;
	case e_exp:
		node->ret->exp = child->exp;
		break;
	default:
		addChildStub(node, child);
		break;
	}
}

void XMLProgParser::start_rettype(Context *node, const char **attr)
{
}

void XMLProgParser::addChildTo_rettype(Context *node, const Context *child) const
{
	node->type = child->type;
}

void XMLProgParser::start_prefreturn(Context *node, const char **attr)
{
	if (phase == 1) {
		return;
	}
}

void XMLProgParser::addChildTo_prefreturn(Context *node, const Context *child) const
{
	if (phase == 1) {
		return;
	}
	node->type = child->type;
}

void XMLProgParser::start_prefparam(Context *node, const char **attr)
{
	if (phase == 1) {
		return;
	}
	const char *n = getAttr(attr, "index");
	assert(n);
	node->n = atoi(n);
}

void XMLProgParser::addChildTo_prefparam(Context *node, const Context *child) const
{
	if (phase == 1) {
		return;
	}
	//switch (child->tag) {
	//default:
		addChildStub(node, child);
		//break;
	//}
}

void XMLProgParser::start_cfg(Context *node, const char **attr)
{
	if (phase == 1) {
		node->cfg = (Cfg *)findId(getAttr(attr, "id"));
		PBB entryBB = (PBB)findId(getAttr(attr, "entryBB"));
		if (entryBB)
			node->cfg->setEntryBB(entryBB);
		PBB exitBB = (PBB)findId(getAttr(attr, "exitBB"));
		if (exitBB)
			node->cfg->setExitBB(exitBB);
		return;
	}
	Cfg *cfg = new Cfg();
	node->cfg = cfg;
	addId(attr, cfg);

	const char *str = getAttr(attr, "wellformed");
	if (str)
		cfg->m_bWellFormed = atoi(str) > 0;
	str = getAttr(attr, "lastLabel");
	if (str)
		cfg->lastLabel = atoi(str);
}

void XMLProgParser::addChildTo_cfg(Context *node, const Context *child) const
{
	if (phase == 1) {
		switch (child->tag) {
		case e_order:
			node->cfg->Ordering.push_back(child->bb);
			break;
		case e_revorder:
			node->cfg->revOrdering.push_back(child->bb);
			break;
		}
		return;
	}
	switch (child->tag) {
	case e_bb:
		node->cfg->addBB(child->bb);
		break;
	case e_order:
		break;
	case e_revorder:
		break;
	default:
		addChildStub(node, child);
		break;
	}
}

void XMLProgParser::start_bb(Context *node, const char **attr)
{
	PBB bb;
	if (phase == 1) {
		bb = node->bb = (BasicBlock *)findId(getAttr(attr, "id"));
		PBB h = (PBB)findId(getAttr(attr, "m_loopHead"));
		if (h)
			bb->m_loopHead = h;
		h = (PBB)findId(getAttr(attr, "m_caseHead"));
		if (h)
			bb->m_caseHead = h;
		h = (PBB)findId(getAttr(attr, "m_condFollow"));
		if (h)
			bb->m_condFollow = h;
		h = (PBB)findId(getAttr(attr, "m_loopFollow"));
		if (h)
			bb->m_loopFollow = h;
		h = (PBB)findId(getAttr(attr, "m_latchNode"));
		if (h)
			bb->m_latchNode = h;
		// note the rediculous duplication here
		h = (PBB)findId(getAttr(attr, "immPDom"));
		if (h)
			bb->immPDom = h;
		h = (PBB)findId(getAttr(attr, "loopHead"));
		if (h)
			bb->loopHead = h;
		h = (PBB)findId(getAttr(attr, "caseHead"));
		if (h)
			bb->caseHead = h;
		h = (PBB)findId(getAttr(attr, "condFollow"));
		if (h)
			bb->condFollow = h;
		h = (PBB)findId(getAttr(attr, "loopFollow"));
		if (h)
			bb->loopFollow = h;
		h = (PBB)findId(getAttr(attr, "latchNode"));
		if (h)
			bb->latchNode = h;
		return;
	}
	bb = new BasicBlock();
	node->bb = bb;
	addId(attr, bb);

	const char *str = getAttr(attr, "nodeType");
	if (str)
		bb->m_nodeType = (BBTYPE)atoi(str);
	str = getAttr(attr, "labelNum");
	if (str)
		bb->m_iLabelNum = atoi(str);
	str = getAttr(attr, "label");
	if (str)
		bb->m_labelStr = strdup(str);
	str = getAttr(attr, "labelneeded");
	if (str)
		bb->m_labelneeded = atoi(str) > 0;
	str = getAttr(attr, "incomplete");
	if (str)
		bb->m_bIncomplete = atoi(str) > 0;
	str = getAttr(attr, "jumpreqd");
	if (str)
		bb->m_bJumpReqd = atoi(str) > 0;
	str = getAttr(attr, "m_traversed");
	if (str)
		bb->m_iTraversed = atoi(str) > 0;
	str = getAttr(attr, "DFTfirst");
	if (str)
		bb->m_DFTfirst = atoi(str);
	str = getAttr(attr, "DFTlast");
	if (str)
		bb->m_DFTlast = atoi(str);
	str = getAttr(attr, "DFTrevfirst");
	if (str)
		bb->m_DFTrevfirst = atoi(str);
	str = getAttr(attr, "DFTrevlast");
	if (str)
		bb->m_DFTrevlast = atoi(str);
	str = getAttr(attr, "structType");
	if (str)
		bb->m_structType = (SBBTYPE)atoi(str);
	str = getAttr(attr, "loopCondType");
	if (str)
		bb->m_loopCondType = (SBBTYPE)atoi(str);
	str = getAttr(attr, "ord");
	if (str)
		bb->ord = atoi(str);
	str = getAttr(attr, "revOrd");
	if (str)
		bb->revOrd = atoi(str);
	str = getAttr(attr, "inEdgesVisited");
	if (str)
		bb->inEdgesVisited = atoi(str);
	str = getAttr(attr, "numForwardInEdges");
	if (str)
		bb->numForwardInEdges = atoi(str);
	str = getAttr(attr, "loopStamp1");
	if (str)
		bb->loopStamps[0] = atoi(str);
	str = getAttr(attr, "loopStamp2");
	if (str)
		bb->loopStamps[1] = atoi(str);
	str = getAttr(attr, "revLoopStamp1");
	if (str)
		bb->revLoopStamps[0] = atoi(str);
	str = getAttr(attr, "revLoopStamp2");
	if (str)
		bb->revLoopStamps[1] = atoi(str);
	str = getAttr(attr, "traversed");
	if (str)
		bb->traversed = (travType)atoi(str);
	str = getAttr(attr, "hllLabel");
	if (str)
		bb->hllLabel = atoi(str) > 0;
	str = getAttr(attr, "labelStr");
	if (str)
		bb->labelStr = strdup(str);
	str = getAttr(attr, "indentLevel");
	if (str)
		bb->indentLevel = atoi(str);
	str = getAttr(attr, "sType");
	if (str)
		bb->sType = (structType)atoi(str);
	str = getAttr(attr, "usType");
	if (str)
		bb->usType = (unstructType)atoi(str);
	str = getAttr(attr, "lType");
	if (str)
		bb->lType = (loopType)atoi(str);
	str = getAttr(attr, "cType");
	if (str)
		bb->cType = (condType)atoi(str);
}

void XMLProgParser::addChildTo_bb(Context *node, const Context *child) const
{
	if (phase == 1) {
		switch (child->tag) {
		case e_inedge:
			node->bb->addInEdge(child->bb);
			break;
		case e_outedge:
			node->bb->addOutEdge(child->bb);
			break;
		}
		return;
	}
	switch (child->tag) {
	case e_inedge:
		break;
	case e_outedge:
		break;
	case e_livein:
		node->bb->addLiveIn((Location *)child->exp);
		break;
	case e_rtl:
		node->bb->addRTL(child->rtl);
		break;
	default:
		addChildStub(node, child);
		break;
	}
}

void XMLProgParser::start_inedge(Context *node, const char **attr)
{
	if (phase == 1)
		node->bb = (BasicBlock *)findId(getAttr(attr, "bb"));
	else
		node->bb = NULL;
}

void XMLProgParser::addChildTo_inedge(Context *node, const Context *child) const
{
	//switch (child->tag) {
	//default:
		addChildStub(node, child);
		//break;
	//}
}

void XMLProgParser::start_outedge(Context *node, const char **attr)
{
	if (phase == 1)
		node->bb = (BasicBlock *)findId(getAttr(attr, "bb"));
	else
		node->bb = NULL;
}

void XMLProgParser::addChildTo_outedge(Context *node, const Context *child) const
{
	//switch (child->tag) {
	//default:
		addChildStub(node, child);
		//break;
	//}
}

void XMLProgParser::start_livein(Context *node, const char **attr)
{
}

void XMLProgParser::addChildTo_livein(Context *node, const Context *child) const
{
	node->exp = child->exp;
}

void XMLProgParser::start_order(Context *node, const char **attr)
{
	if (phase == 1)
		node->bb = (PBB)findId(getAttr(attr, "bb"));
	else
		node->bb = NULL;
}

void XMLProgParser::addChildTo_order(Context *node, const Context *child) const
{
	//switch (child->tag) {
	//default:
		addChildStub(node, child);
		//break;
	//}
}

void XMLProgParser::start_revorder(Context *node, const char **attr)
{
	if (phase == 1)
		node->bb = (PBB)findId(getAttr(attr, "bb"));
	else
		node->bb = NULL;
}

void XMLProgParser::addChildTo_revorder(Context *node, const Context *child) const
{
	//switch (child->tag) {
	//default:
		addChildStub(node, child);
		//break;
	//}
}

void XMLProgParser::start_rtl(Context *node, const char **attr)
{
	if (phase == 1) {
		node->rtl = (RTL *)findId(getAttr(attr, "id"));
		return;
	}
	node->rtl = new RTL();
	addId(attr, node->rtl);
	const char *a = getAttr(attr, "addr");
	if (a)
		node->rtl->nativeAddr = atoi(a);
}

void XMLProgParser::addChildTo_rtl(Context *node, const Context *child) const
{
	if (phase == 1) {
		return;
	}
	switch (child->tag) {
	case e_stmt:
		node->rtl->appendStmt(child->stmt);
		break;
	default:
		addChildStub(node, child);
		break;
	}
}

void XMLProgParser::start_stmt(Context *node, const char **attr)
{
}

void XMLProgParser::addChildTo_stmt(Context *node, const Context *child) const
{
	node->stmt = child->stmt;
}

void XMLProgParser::start_assign(Context *node, const char **attr)
{
	if (phase == 1) {
		node->stmt = (Statement *)findId(getAttr(attr, "id"));
		UserProc *p = (UserProc *)findId(getAttr(attr, "proc"));
		if (p)
			node->stmt->setProc(p);
		Statement *parent = (Statement *)findId(getAttr(attr, "parent"));
		if (parent)
			node->stmt->parent = parent;
		return;
	}
	node->stmt = new Assign();
	addId(attr, node->stmt);
	const char *n = getAttr(attr, "number");
	if (n)
		node->stmt->number = atoi(n);
}

void XMLProgParser::addChildTo_assign(Context *node, const Context *child) const
{
	if (phase == 1) {
		return;
	}
	Assign *assign = dynamic_cast<Assign *>(node->stmt);
	assert(assign);
	switch (child->tag) {
	case e_lhs:
		assign->setLeft(child->exp);
		break;
	case e_rhs:
		assign->setRight(child->exp);
		break;
	case e_type:
		assert(child->type);
		assign->setType(child->type);
		break;
	default:
		addChildStub(node, child);
		break;
	}
}

void XMLProgParser::start_assignment(Context *node, const char **attr)
{
}

void XMLProgParser::addChildTo_assignment(Context *node, const Context *child) const
{
	node->stmt = child->stmt;
}

void XMLProgParser::start_phiassign(Context *node, const char **attr)
{
	// FIXME: TBC
}

void XMLProgParser::addChildTo_phiassign(Context *node, const Context *child) const
{
	if (phase == 1) {
		return;
	}
	PhiAssign *pa = dynamic_cast<PhiAssign *>(node->stmt);
	assert(pa);
	switch (child->tag) {
	case e_lhs:
		pa->setLeft(child->exp);
		break;
	// FIXME: More required
	default:
		addChildStub(node, child);
		break;
	}
}

void XMLProgParser::start_lhs(Context *node, const char **attr)
{
}

void XMLProgParser::addChildTo_lhs(Context *node, const Context *child) const
{
	node->exp = child->exp;
}

void XMLProgParser::start_rhs(Context *node, const char **attr)
{
}

void XMLProgParser::addChildTo_rhs(Context *node, const Context *child) const
{
	node->exp = child->exp;
}

void XMLProgParser::start_callstmt(Context *node, const char **attr)
{
	if (phase == 1) {
		node->stmt = (Statement *)findId(getAttr(attr, "id"));
		UserProc *p = (UserProc *)findId(getAttr(attr, "proc"));
		if (p)
			((Statement *)node->stmt)->setProc(p);
		Statement *s = (Statement *)findId(getAttr(attr, "parent"));
		if (s)
			((Statement *)node->stmt)->parent = s;
		return;
	}
	CallStatement *call = new CallStatement();
	node->stmt = call;
	addId(attr, call);
	const char *n = getAttr(attr, "number");
	if (n)
		call->number = atoi(n);
	n = getAttr(attr, "computed");
	if (n)
		call->m_isComputed = atoi(n) > 0;
	n = getAttr(attr, "returnAftercall");
	if (n)
		call->returnAfterCall = atoi(n) > 0;
}

void XMLProgParser::addChildTo_callstmt(Context *node, const Context *child) const
{
	CallStatement *call = dynamic_cast<CallStatement *>(node->stmt);
	assert(call);
	if (phase == 1) {
		switch (child->tag) {
		case e_dest:
			if (child->proc)
				call->setDestProc(child->proc);
			break;
		}
		return;
	}
	Exp *returnExp = NULL;
	switch (child->tag) {
	case e_dest:
		call->setDest(child->exp);
		break;
	case e_argument:
		call->appendArgument((Assignment *)child->stmt);
		break;
	case e_returnexp:
		// Assume that the corresponding return type will appear next
		returnExp = child->exp;
		break;
	default:
		addChildStub(node, child);
		break;
	}
}

void XMLProgParser::start_dest(Context *node, const char **attr)
{
	if (phase == 1) {
		Proc *p = (Proc *)findId(getAttr(attr, "proc"));
		if (p)
			node->proc = p;
		return;
	}
}

void XMLProgParser::addChildTo_dest(Context *node, const Context *child) const
{
	node->exp = child->exp;
}

void XMLProgParser::start_argument(Context *node, const char **attr)
{
}

void XMLProgParser::addChildTo_argument(Context *node, const Context *child) const
{
	node->exp = child->exp;
}

void XMLProgParser::start_returnexp(Context *node, const char **attr)
{
}

void XMLProgParser::addChildTo_returnexp(Context *node, const Context *child) const
{
	node->exp = child->exp;
}

void XMLProgParser::start_returntype(Context *node, const char **attr)
{
}

void XMLProgParser::addChildTo_returntype(Context *node, const Context *child) const
{
	node->type = child->type;
}

void XMLProgParser::start_returnstmt(Context *node, const char **attr)
{
	if (phase == 1) {
		node->stmt = (Statement *)findId(getAttr(attr, "id"));
		UserProc *p = (UserProc *)findId(getAttr(attr, "proc"));
		if (p)
			((Statement *)node->stmt)->setProc(p);
		Statement *s = (Statement *)findId(getAttr(attr, "parent"));
		if (s)
			((Statement *)node->stmt)->parent = s;
		return;
	}
	ReturnStatement *ret = new ReturnStatement();
	node->stmt = ret;
	addId(attr, ret);
	const char *n = getAttr(attr, "number");
	if (n)
		ret->number = atoi(n);
	n = getAttr(attr, "retAddr");
	if (n)
		ret->retAddr = atoi(n);
}

void XMLProgParser::addChildTo_returnstmt(Context *node, const Context *child) const
{
	ReturnStatement *ret = dynamic_cast<ReturnStatement *>(node->stmt);
	assert(ret);
	if (phase == 1) {
		return;
	}
	switch (child->tag) {
	case e_modifieds:
		ret->modifieds.append((Assignment *)child->stmt);
		break;
	case e_returns:
		ret->returns.append((Assignment *)child->stmt);
		break;
	default:
		addChildStub(node, child);
		break;
	}
}

void XMLProgParser::start_returns(Context *node, const char **attr)
{
}

void XMLProgParser::addChildTo_returns(Context *node, const Context *child) const
{
}

void XMLProgParser::start_modifieds(Context *node, const char **attr)
{
}

void XMLProgParser::addChildTo_modifieds(Context *node, const Context *child) const
{
}

void XMLProgParser::start_gotostmt(Context *node, const char **attr)
{
	if (phase == 1) {
		node->stmt = (Statement *)findId(getAttr(attr, "id"));
		UserProc *p = (UserProc *)findId(getAttr(attr, "proc"));
		if (p)
			((Statement *)node->stmt)->setProc(p);
		Statement *s = (Statement *)findId(getAttr(attr, "parent"));
		if (s)
			((Statement *)node->stmt)->parent = s;
		return;
	}
	GotoStatement *branch = new GotoStatement();
	node->stmt = branch;
	addId(attr, branch);
	const char *n = getAttr(attr, "number");
	if (n)
		branch->number = atoi(n);
	n = getAttr(attr, "computed");
	if (n)
		branch->m_isComputed = atoi(n) > 0;
}

void XMLProgParser::addChildTo_gotostmt(Context *node, const Context *child) const
{
	GotoStatement *branch = dynamic_cast<GotoStatement *>(node->stmt);
	assert(branch);
	if (phase == 1) {
		return;
	}
	switch (child->tag) {
	case e_dest:
		branch->setDest(child->exp);
		break;
	default:
		addChildStub(node, child);
		break;
	}
}

void XMLProgParser::start_branchstmt(Context *node, const char **attr)
{
	if (phase == 1) {
		node->stmt = (Statement *)findId(getAttr(attr, "id"));
		UserProc *p = (UserProc *)findId(getAttr(attr, "proc"));
		if (p)
			((Statement *)node->stmt)->setProc(p);
		Statement *s = (Statement *)findId(getAttr(attr, "parent"));
		if (s)
			((Statement *)node->stmt)->parent = s;
		return;
	}
	BranchStatement *branch = new BranchStatement();
	node->stmt = branch;
	addId(attr, branch);
	const char *n = getAttr(attr, "number");
	if (n)
		branch->number = atoi(n);
	n = getAttr(attr, "computed");
	if (n)
		branch->m_isComputed = atoi(n) > 0;
	n = getAttr(attr, "jtcond");
	if (n)
		branch->jtCond = (BRANCH_TYPE)atoi(n);
	n = getAttr(attr, "float");
	if (n)
		branch->bFloat = atoi(n) > 0;
}

void XMLProgParser::addChildTo_branchstmt(Context *node, const Context *child) const
{
	BranchStatement *branch = dynamic_cast<BranchStatement *>(node->stmt);
	assert(branch);
	if (phase == 1) {
		return;
	}
	switch (child->tag) {
	case e_cond:
		branch->setCondExpr(child->exp);
		break;
	case e_dest:
		branch->setDest(child->exp);
		break;
	default:
		addChildStub(node, child);
		break;
	}
}

void XMLProgParser::start_cond(Context *node, const char **attr)
{
}

void XMLProgParser::addChildTo_cond(Context *node, const Context *child) const
{
	node->exp = child->exp;
}

void XMLProgParser::start_casestmt(Context *node, const char **attr)
{
	if (phase == 1) {
		node->stmt = (Statement *)findId(getAttr(attr, "id"));
		UserProc *p = (UserProc *)findId(getAttr(attr, "proc"));
		if (p)
			((Statement *)node->stmt)->setProc(p);
		Statement *s = (Statement *)findId(getAttr(attr, "parent"));
		if (s)
			((Statement *)node->stmt)->parent = s;
		return;
	}
	CaseStatement *cas = new CaseStatement();
	node->stmt = cas;
	addId(attr, cas);
	const char *n = getAttr(attr, "number");
	if (n)
		cas->number = atoi(n);
	n = getAttr(attr, "computed");
	if (n)
		cas->m_isComputed = atoi(n) > 0;
}

void XMLProgParser::addChildTo_casestmt(Context *node, const Context *child) const
{
	CaseStatement *cas = dynamic_cast<CaseStatement *>(node->stmt);
	assert(cas);
	if (phase == 1) {
		return;
	}
	switch (child->tag) {
	case e_dest:
		cas->setDest(child->exp);
		break;
	default:
		addChildStub(node, child);
		break;
	}
}

void XMLProgParser::start_boolasgn(Context *node, const char **attr)
{
	if (phase == 1) {
		node->stmt = (Statement *)findId(getAttr(attr, "id"));
		UserProc *p = (UserProc *)findId(getAttr(attr, "proc"));
		if (p)
			((Statement *)node->stmt)->setProc(p);
		Statement *s = (Statement *)findId(getAttr(attr, "parent"));
		if (s)
			((Statement *)node->stmt)->parent = s;
		return;
	}
	const char *n = getAttr(attr, "size");
	assert(n);
	BoolAssign *boo = new BoolAssign(atoi(n));
	node->stmt = boo;
	addId(attr, boo);
	n = getAttr(attr, "number");
	if (n)
		boo->number = atoi(n);
	n = getAttr(attr, "jtcond");
	if (n)
		boo->jtCond = (BRANCH_TYPE)atoi(n);
	n = getAttr(attr, "float");
	if (n)
		boo->bFloat = atoi(n) > 0;
}

void XMLProgParser::addChildTo_boolasgn(Context *node, const Context *child) const
{
	BoolAssign *boo = dynamic_cast<BoolAssign *>(node->stmt);
	assert(boo);
	if (phase == 1) {
		return;
	}
	switch (child->tag) {
	case e_cond:
		boo->pCond = child->exp;
		break;
	case e_lhs:
		boo->lhs = child->exp;
		break;
	default:
		addChildStub(node, child);
		break;
	}
}

void XMLProgParser::start_type(Context *node, const char **attr)
{
}

void XMLProgParser::addChildTo_type(Context *node, const Context *child) const
{
	node->type = child->type;
}

void XMLProgParser::start_exp(Context *node, const char **attr)
{
}

void XMLProgParser::addChildTo_exp(Context *node, const Context *child) const
{
	node->exp = child->exp;
}

void XMLProgParser::start_voidtype(Context *node, const char **attr)
{
	if (phase == 1) {
		node->type = (Type *)findId(getAttr(attr, "id"));
		return;
	}
	node->type = new VoidType();
	addId(attr, node->type);
}

void XMLProgParser::addChildTo_voidtype(Context *node, const Context *child) const
{
	//switch (child->tag) {
	//default:
		addChildStub(node, child);
		//break;
	//}
}

void XMLProgParser::start_integertype(Context *node, const char **attr)
{
	if (phase == 1) {
		node->type = (Type *)findId(getAttr(attr, "id"));
		return;
	}
	IntegerType *ty = new IntegerType();
	node->type = ty;
	addId(attr, ty);
	const char *n = getAttr(attr, "size");
	if (n)
		ty->size = atoi(n);
	n = getAttr(attr, "signedness");
	if (n)
		ty->signedness = atoi(n);
}

void XMLProgParser::addChildTo_integertype(Context *node, const Context *child) const
{
	//switch (child->tag) {
	//default:
		addChildStub(node, child);
		//break;
	//}
}

void XMLProgParser::start_pointertype(Context *node, const char **attr)
{
	if (phase == 1) {
		node->type = (Type *)findId(getAttr(attr, "id"));
		return;
	}
	node->type = new PointerType(NULL);
	addId(attr, node->type);
}

void XMLProgParser::addChildTo_pointertype(Context *node, const Context *child) const
{
	PointerType *p = dynamic_cast<PointerType *>(node->type);
	assert(p);
	if (phase == 1) {
		return;
	}
	p->setPointsTo(child->type);
}

void XMLProgParser::start_chartype(Context *node, const char **attr)
{
	if (phase == 1) {
		node->type = (Type *)findId(getAttr(attr, "id"));
		return;
	}
	node->type = new CharType();
	addId(attr, node->type);
}

void XMLProgParser::addChildTo_chartype(Context *node, const Context *child) const
{
	//switch (child->tag) {
	//default:
		addChildStub(node, child);
		//break;
	//}
}

void XMLProgParser::start_namedtype(Context *node, const char **attr)
{
	if (phase == 1) {
		node->type = (Type *)findId(getAttr(attr, "id"));
		return;
	}
	node->type = new NamedType(getAttr(attr, "name"));
	addId(attr, node->type);
}

void XMLProgParser::addChildTo_namedtype(Context *node, const Context *child) const
{
	//switch (child->tag) {
	//default:
		addChildStub(node, child);
		//break;
	//}
}

void XMLProgParser::start_arraytype(Context *node, const char **attr)
{
	if (phase == 1) {
		node->type = (Type *)findId(getAttr(attr, "id"));
		return;
	}
	ArrayType *a = new ArrayType();
	node->type = a;
	addId(attr, a);
	const char *len = getAttr(attr, "length");
	if (len)
		a->length = atoi(len);
}

void XMLProgParser::addChildTo_arraytype(Context *node, const Context *child) const
{
	ArrayType *a = dynamic_cast<ArrayType *>(node->type);
	assert(a);
	switch (child->tag) {
	case e_basetype:
		a->base_type = child->type;
		break;
	default:
		addChildStub(node, child);
		break;
	}
}

void XMLProgParser::start_basetype(Context *node, const char **attr)
{
}

void XMLProgParser::addChildTo_basetype(Context *node, const Context *child) const
{
	node->type = child->type;
}

void XMLProgParser::start_sizetype(Context *node, const char **attr)
{
	if (phase == 1) {
		node->type = (Type *)findId(getAttr(attr, "id"));
		return;
	}
	SizeType *ty = new SizeType();
	node->type = ty;
	addId(attr, ty);
	const char *n = getAttr(attr, "size");
	if (n)
		ty->size = atoi(n);
}

void XMLProgParser::addChildTo_sizetype(Context *node, const Context *child) const
{
	//switch (child->tag) {
	//default:
		addChildStub(node, child);
		//break;
	//}
}

void XMLProgParser::start_location(Context *node, const char **attr)
{
	if (phase == 1) {
		node->exp = (Exp *)findId(getAttr(attr, "id"));
		UserProc *p = (UserProc *)findId(getAttr(attr, "proc"));
		if (p)
			((Location *)node->exp)->setProc(p);
		return;
	}
	OPER op = (OPER)operFromString(getAttr(attr, "op"));
	assert(op != -1);
	node->exp = new Location(op);
	addId(attr, node->exp);
}

void XMLProgParser::addChildTo_location(Context *node, const Context *child) const
{
	if (phase == 1) {
		return;
	}
	Location *l = dynamic_cast<Location *>(node->exp);
	assert(l);
	switch (child->tag) {
	case e_subexp1:
		node->exp->setSubExp1(child->exp);
		break;
	default:
		addChildStub(node, child);
		break;
	}
}

void XMLProgParser::start_unary(Context *node, const char **attr)
{
	if (phase == 1) {
		node->exp = (Exp *)findId(getAttr(attr, "id"));
		return;
	}
	OPER op = (OPER)operFromString(getAttr(attr, "op"));
	assert(op != -1);
	node->exp = new Unary(op);
	addId(attr, node->exp);
}

void XMLProgParser::addChildTo_unary(Context *node, const Context *child) const
{
	if (phase == 1) {
		return;
	}
	switch (child->tag) {
	case e_subexp1:
		node->exp->setSubExp1(child->exp);
		break;
	default:
		addChildStub(node, child);
		break;
	}
}

void XMLProgParser::start_binary(Context *node, const char **attr)
{
	if (phase == 1) {
		node->exp = (Exp *)findId(getAttr(attr, "id"));
		return;
	}
	OPER op = (OPER)operFromString(getAttr(attr, "op"));
	assert(op != -1);
	node->exp = new Binary(op);
	addId(attr, node->exp);
}

void XMLProgParser::addChildTo_binary(Context *node, const Context *child) const
{
	if (phase == 1) {
		return;
	}
	switch (child->tag) {
	case e_subexp1:
		node->exp->setSubExp1(child->exp);
		break;
	case e_subexp2:
		node->exp->setSubExp2(child->exp);
		break;
	default:
		addChildStub(node, child);
		break;
	}
}

void XMLProgParser::start_ternary(Context *node, const char **attr)
{
	if (phase == 1) {
		node->exp = (Exp *)findId(getAttr(attr, "id"));
		return;
	}
	OPER op = (OPER)operFromString(getAttr(attr, "op"));
	assert(op != -1);
	node->exp = new Ternary(op);
	addId(attr, node->exp);
}

void XMLProgParser::addChildTo_ternary(Context *node, const Context *child) const
{
	if (phase == 1) {
		return;
	}
	switch (child->tag) {
	case e_subexp1:
		node->exp->setSubExp1(child->exp);
		break;
	case e_subexp2:
		node->exp->setSubExp2(child->exp);
		break;
	case e_subexp3:
		node->exp->setSubExp3(child->exp);
		break;
	default:
		addChildStub(node, child);
		break;
	}
}

void XMLProgParser::start_const(Context *node, const char **attr)
{
	if (phase == 1) {
		node->exp = (Exp *)findId(getAttr(attr, "id"));
		return;
	}
	double d;
	const char *value = getAttr(attr, "value");
	const char *opstring = getAttr(attr, "op");
	assert(value);
	assert(opstring);
	//std::cerr << "got value=" << value << " opstring=" << opstring << "\n";
	OPER op = (OPER)operFromString(opstring);
	assert(op != -1);
	switch (op) {
	case opIntConst:
		node->exp = new Const(atoi(value));
		addId(attr, node->exp);
		break;
	case opStrConst:
		node->exp = new Const(strdup(value));
		addId(attr, node->exp);
		break;
	case opFltConst:
		sscanf(value, "%lf", &d);
		node->exp = new Const(d);
		addId(attr, node->exp);
		break;
	default:
		LOG << "unknown Const op " << op << "\n";
		assert(false);
	}
	//std::cerr << "end of start const\n";
}

void XMLProgParser::addChildTo_const(Context *node, const Context *child) const
{
	//switch (child->tag) {
	//default:
		addChildStub(node, child);
		//break;
	//}
}

void XMLProgParser::start_terminal(Context *node, const char **attr)
{
	if (phase == 1) {
		node->exp = (Exp *)findId(getAttr(attr, "id"));
		return;
	}
	OPER op = (OPER)operFromString(getAttr(attr, "op"));
	assert(op != -1);
	node->exp = new Terminal(op);
	addId(attr, node->exp);
}

void XMLProgParser::addChildTo_terminal(Context *node, const Context *child) const
{
	//switch (child->tag) {
	//default:
		addChildStub(node, child);
		//break;
	//}
}

void XMLProgParser::start_typedexp(Context *node, const char **attr)
{
	if (phase == 1) {
		node->exp = (Exp *)findId(getAttr(attr, "id"));
		return;
	}
	node->exp = new TypedExp();
	addId(attr, node->exp);
}

void XMLProgParser::addChildTo_typedexp(Context *node, const Context *child) const
{
	TypedExp *t = dynamic_cast<TypedExp *>(node->exp);
	assert(t);
	switch (child->tag) {
	case e_type:
		t->type = child->type;
		break;
	case e_subexp1:
		node->exp->setSubExp1(child->exp);
		break;
	default:
		addChildStub(node, child);
		break;
	}
}

void XMLProgParser::start_refexp(Context *node, const char **attr)
{
	if (phase == 1) {
		node->exp = (Exp *)findId(getAttr(attr, "id"));
		RefExp *r = dynamic_cast<RefExp *>(node->exp);
		assert(r);
		r->def = (Statement *)findId(getAttr(attr, "def"));
		return;
	}
	node->exp = new RefExp();
	addId(attr, node->exp);
}

void XMLProgParser::addChildTo_refexp(Context *node, const Context *child) const
{
	switch (child->tag) {
	case e_subexp1:
		node->exp->setSubExp1(child->exp);
		break;
	default:
		addChildStub(node, child);
		break;
	}
}

void XMLProgParser::start_def(Context *node, const char **attr)
{
	if (phase == 1) {
		node->stmt = (Statement *)findId(getAttr(attr, "stmt"));
		return;
	}
}

void XMLProgParser::addChildTo_def(Context *node, const Context *child) const
{
	//switch (child->tag) {
	//default:
		addChildStub(node, child);
		//break;
	//}
}

void XMLProgParser::start_subexp1(Context *node, const char **attr)
{
}

void XMLProgParser::addChildTo_subexp1(Context *node, const Context *child) const
{
	node->exp = child->exp;
}

void XMLProgParser::start_subexp2(Context *node, const char **attr)
{
}

void XMLProgParser::addChildTo_subexp2(Context *node, const Context *child) const
{
	node->exp = child->exp;
}

void XMLProgParser::start_subexp3(Context *node, const char **attr)
{
}

void XMLProgParser::addChildTo_subexp3(Context *node, const Context *child) const
{
	node->exp = child->exp;
}


void XMLProgParser::persistToXML(Prog *prog)
{
	prog->m_rootCluster->openStreams("xml");
	std::ofstream &os = prog->m_rootCluster->getStream();
	os << "<prog path=\"" << prog->getPath()
	   << "\" name=\"" << prog->getName()
	   << "\" iNumberedProc=\"" << prog->m_iNumberedProc
	   << "\">\n";
	for (std::set<Global *>::iterator it1 = prog->globals.begin(); it1 != prog->globals.end(); it1++)
		persistToXML(os, *it1);
	persistToXML(os, prog->m_rootCluster);
	for (std::list<Proc *>::iterator it = prog->m_procs.begin(); it != prog->m_procs.end(); it++) {
		Proc *p = *it;
		persistToXML(p->getCluster()->getStream(), p);
	}
	os << "</prog>\n";
	os.close();
	prog->m_rootCluster->closeStreams();
}

void XMLProgParser::persistToXML(std::ostream &out, Global *g)
{
	out << "<global name=\"" << g->nam
	    << "\" uaddr=\"" << (int)g->uaddr
	    << "\">\n";
	out << "<type>\n";
	persistToXML(out, g->type);
	out << "</type>\n";
	out << "</global>\n";
}

void XMLProgParser::persistToXML(std::ostream &out, Cluster *c)
{
	out << "<cluster id=\"" << (int)c
	    << "\" name=\"" << c->name
	    << "\">\n";
	for (unsigned i = 0; i < c->children.size(); i++) {
		persistToXML(out, c->children[i]);
	}
	out << "</cluster>\n";
}

void XMLProgParser::persistToXML(std::ostream &out, Proc *proc)
{
	if (proc->isLib())
		persistToXML(out, (LibProc *)proc);
	else
		persistToXML(out, (UserProc *)proc);
}

void XMLProgParser::persistToXML(std::ostream &out, LibProc *proc)
{
	out << "<libproc id=\"" << (int)proc
	    << "\" address=\"" << (int)proc->address
	    << "\" firstCallerAddress=\"" << proc->m_firstCallerAddr;
	if (proc->m_firstCaller)
		out << "\" firstCaller=\"" << (int)proc->m_firstCaller;
	if (proc->cluster)
		out << "\" cluster=\"" << (int)proc->cluster;
	out << "\">\n";

	persistToXML(out, proc->signature);

	for (std::set<CallStatement *>::iterator it = proc->callerSet.begin(); it != proc->callerSet.end(); it++)
		out << "<caller call=\"" << (int)(*it) << "\"/>\n";
	for (std::map<Exp *, Exp *, lessExpStar>::iterator it = proc->provenTrue.begin(); it != proc->provenTrue.end(); it++) {
		out << "<proven_true>\n";
		persistToXML(out, it->first);
		persistToXML(out, it->second);
		out << "</proven_true>\n";
	}
	out << "</libproc>\n";
}

void XMLProgParser::persistToXML(std::ostream &out, UserProc *proc)
{
	out << "<userproc id=\"" << (int)proc
	    << "\" address=\"" << (int)proc->address
	    << "\" status=\"" << (int)proc->status
	    << "\" firstCallerAddress=\"" << proc->m_firstCallerAddr;
	if (proc->m_firstCaller)
		out << "\" firstCaller=\"" << (int)proc->m_firstCaller;
	if (proc->cluster)
		out << "\" cluster=\"" << (int)proc->cluster;
	if (proc->theReturnStatement)
		out << "\" retstmt=\"" << (int)proc->theReturnStatement;
	out << "\">\n";

	persistToXML(out, proc->signature);

	for (std::set<CallStatement *>::iterator it = proc->callerSet.begin(); it != proc->callerSet.end(); it++)
		out << "<caller call=\"" << (int)(*it) << "\"/>\n";
	for (std::map<Exp *, Exp *, lessExpStar>::iterator it = proc->provenTrue.begin(); it != proc->provenTrue.end(); it++) {
		out << "<proven_true>\n";
		persistToXML(out, it->first);
		persistToXML(out, it->second);
		out << "</proven_true>\n";
	}

	for (std::map<std::string, Type *>::iterator it1 = proc->locals.begin(); it1 != proc->locals.end(); it1++) {
		out << "<local name=\"" << (*it1).first << "\">\n";
		out << "<type>\n";
		persistToXML(out, (*it1).second);
		out << "</type>\n";
		out << "</local>\n";
	}

	for (std::multimap<Exp *, Exp *, lessExpStar>::iterator it2 = proc->symbolMap.begin(); it2 != proc->symbolMap.end(); it2++) {
		out << "<symbol>\n";
		out << "<exp>\n";
		persistToXML(out, (*it2).first);
		out << "</exp>\n";
		out << "<secondexp>\n";
		persistToXML(out, (*it2).second);
		out << "</secondexp>\n";
		out << "</symbol>\n";
	}

	for (std::list<Proc *>::iterator it = proc->calleeList.begin(); it != proc->calleeList.end(); it++)
		out << "<callee proc=\"" << (int)(*it) << "\"/>\n";

	persistToXML(out, proc->cfg);

	out << "</userproc>\n";
}

void XMLProgParser::persistToXML(std::ostream &out, Signature *sig)
{
	out << "<signature id=\"" << (int)sig
	    << "\" name=\"" << sig->name
	    << "\" ellipsis=\"" << (int)sig->ellipsis
	    << "\" preferedName=\"" << sig->preferedName;
	if (sig->getPlatform() != PLAT_GENERIC)
		out << "\" platform=\"" << sig->platformName(sig->getPlatform());
	if (sig->getConvention() != CONV_NONE)
		out << "\" convention=\"" << sig->conventionName(sig->getConvention());
	out << "\">\n";
	for (unsigned i = 0; i < sig->params.size(); i++) {
		out << "<param id=\"" << (int)sig->params[i]
		    << "\" name=\"" << sig->params[i]->getName()
		    << "\">\n";
		out << "<type>\n";
		persistToXML(out, sig->params[i]->getType());
		out << "</type>\n";
		out << "<exp>\n";
		persistToXML(out, sig->params[i]->getExp());
		out << "</exp>\n";
		out << "</param>\n";
	}
	for (Returns::iterator rr = sig->returns.begin(); rr != sig->returns.end(); ++rr) {
		out << "<return>\n";
		out << "<type>\n";
		persistToXML(out, (*rr)->type);
		out << "</type>\n";
		out << "<exp>\n";
		persistToXML(out, (*rr)->exp);
		out << "</exp>\n";
		out << "</return>\n";
	}
	if (sig->rettype) {
		out << "<rettype>\n";
		persistToXML(out, sig->rettype);
		out << "</rettype>\n";
	}
	if (sig->preferedReturn) {
		out << "<prefreturn>\n";
		persistToXML(out, sig->preferedReturn);
		out << "</prefreturn>\n";
	}
	for (unsigned i = 0; i < sig->preferedParams.size(); i++)
		out << "<prefparam index=\"" << sig->preferedParams[i] << "\"/>\n";
	out << "</signature>\n";
}

void XMLProgParser::persistToXML(std::ostream &out, Type *ty)
{
	VoidType *v = dynamic_cast<VoidType *>(ty);
	if (v) {
		out << "<voidtype id=\"" << (int)ty << "\"/>\n";
		return;
	}
	FuncType *f = dynamic_cast<FuncType *>(ty);
	if (f) {
		out << "<functype id=\"" << (int)ty << "\">\n";
		persistToXML(out, f->signature);
		out << "</functype>\n";
		return;
	}
	IntegerType *i = dynamic_cast<IntegerType *>(ty);
	if (i) {
		out << "<integertype id=\"" << (int)ty
		    << "\" size=\"" << i->size
		    << "\" signedness=\"" << i->signedness
		    << "\"/>\n";
		return;
	}
	FloatType *fl = dynamic_cast<FloatType *>(ty);
	if (fl) {
		out << "<floattype id=\"" << (int)ty
		    << "\" size=\"" << fl->size << "\"/>\n";
		return;
	}
	BooleanType *b = dynamic_cast<BooleanType *>(ty);
	if (b) {
		out << "<booleantype id=\"" << (int)ty << "\"/>\n";
		return;
	}
	CharType *c = dynamic_cast<CharType *>(ty);
	if (c) {
		out << "<chartype id=\"" << (int)ty << "\"/>\n";
		return;
	}
	PointerType *p = dynamic_cast<PointerType *>(ty);
	if (p) {
		out << "<pointertype id=\"" << (int)ty << "\">\n";
		persistToXML(out, p->points_to);
		out << "</pointertype>\n";
		return;
	}
	ArrayType *a = dynamic_cast<ArrayType *>(ty);
	if (a) {
		out << "<arraytype id=\"" << (int)ty
		    << "\" length=\"" << (int)a->length
		    << "\">\n";
		out << "<basetype>\n";
		persistToXML(out, a->base_type);
		out << "</basetype>\n";
		out << "</arraytype>\n";
		return;
	}
	NamedType *n = dynamic_cast<NamedType *>(ty);
	if (n) {
		out << "<namedtype id=\"" << (int)ty
		    << "\" name=\"" << n->name
		    << "\"/>\n";
		return;
	}
	CompoundType *co = dynamic_cast<CompoundType *>(ty);
	if (co) {
		out << "<compoundtype id=\"" << (int)ty << "\">\n";
		for (unsigned i = 0; i < co->names.size(); i++) {
			out << "<member name=\"" << co->names[i] << "\">\n";
			persistToXML(out, co->types[i]);
			out << "</member>\n";
		}
		out << "</compoundtype>\n";
		return;
	}
	SizeType *sz = dynamic_cast<SizeType *>(ty);
	if (sz) {
		out << "<sizetype id=\"" << (int)ty
		    << "\" size=\"" << sz->getSize()
		    << "\"/>\n";
		return;
	}
	std::cerr << "unknown type in persistToXML\n";
	assert(false);
}

void XMLProgParser::persistToXML(std::ostream &out, Exp *e)
{
	TypeVal *t = dynamic_cast<TypeVal *>(e);
	if (t) {
		out << "<typeval id=\"" << (int)e
		    << "\" op=\"" << operStrings[t->op]
		    << "\">\n";
		out << "<type>\n";
		persistToXML(out, t->val);
		out << "</type>\n";
		out << "</typeval>\n";
		return;
	}
	Terminal *te = dynamic_cast<Terminal *>(e);
	if (te) {
		out << "<terminal id=\"" << (int)e
		    << "\" op=\"" << operStrings[te->op]
		    << "\"/>\n";
		return;
	}
	Const *c = dynamic_cast<Const *>(e);
	if (c) {
		out << "<const id=\"" << (int)e
		    << "\" op=\"" << operStrings[c->op]
		    << "\" conscript=\"" << c->conscript;
		if (c->op == opIntConst)
			out << "\" value=\"" << c->u.i;
		else if (c->op == opFuncConst)
			out << "\" value=\"" << c->u.a;
		else if (c->op == opFltConst)
			out << "\" value=\"" << c->u.d;
		else if (c->op == opStrConst)
			out << "\" value=\"" << c->u.p;
		else {
			// TODO
			// QWord ll;
			// Proc *pp;
			assert(false);
		}
		out << "\"/>\n";
		return;
	}
	Location *l = dynamic_cast<Location *>(e);
	if (l) {
		out << "<location id=\"" << (int)e;
		if (l->proc)
			out << "\" proc=\"" << (int)l->proc;
		out << "\" op=\"" << operStrings[l->op]
		    << "\">\n";
		out << "<subexp1>\n";
		persistToXML(out, l->subExp1);
		out << "</subexp1>\n";
		out << "</location>\n";
		return;
	}
	RefExp *r = dynamic_cast<RefExp *>(e);
	if (r) {
		out << "<refexp id=\"" << (int)e;
		if (r->def)
			out << "\" def=\"" << (int)r->def;
		out << "\" op=\"" << operStrings[r->op]
		    << "\">\n";
		out << "<subexp1>\n";
		persistToXML(out, r->subExp1);
		out << "</subexp1>\n";
		out << "</refexp>\n";
		return;
	}
	FlagDef *f = dynamic_cast<FlagDef *>(e);
	if (f) {
		out << "<flagdef id=\"" << (int)e;
		if (f->rtl)
			out << "\" rtl=\"" << (int)f->rtl;
		out << "\" op=\"" << operStrings[f->op]
		    << "\">\n";
		out << "<subexp1>\n";
		persistToXML(out, f->subExp1);
		out << "</subexp1>\n";
		out << "</flagdef>\n";
		return;
	}
	TypedExp *ty = dynamic_cast<TypedExp *>(e);
	if (ty) {
		out << "<typedexp id=\"" << (int)e
		    << "\" op=\"" << operStrings[ty->op]
		    << "\">\n";
		out << "<subexp1>\n";
		persistToXML(out, ty->subExp1);
		out << "</subexp1>\n";
		out << "<type>\n";
		persistToXML(out, ty->type);
		out << "</type>\n";
		out << "</typedexp>\n";
		return;
	}
	Ternary *tn = dynamic_cast<Ternary *>(e);
	if (tn) {
		out << "<ternary id=\"" << (int)e
		    << "\" op=\"" << operStrings[tn->op]
		    << "\">\n";
		out << "<subexp1>\n";
		persistToXML(out, tn->subExp1);
		out << "</subexp1>\n";
		out << "<subexp2>\n";
		persistToXML(out, tn->subExp2);
		out << "</subexp2>\n";
		out << "<subexp3>\n";
		persistToXML(out, tn->subExp3);
		out << "</subexp3>\n";
		out << "</ternary>\n";
		return;
	}
	Binary *b = dynamic_cast<Binary *>(e);
	if (b) {
		out << "<binary id=\"" << (int)e
		    << "\" op=\"" << operStrings[b->op]
		    << "\">\n";
		out << "<subexp1>\n";
		persistToXML(out, b->subExp1);
		out << "</subexp1>\n";
		out << "<subexp2>\n";
		persistToXML(out, b->subExp2);
		out << "</subexp2>\n";
		out << "</binary>\n";
		return;
	}
	Unary *u = dynamic_cast<Unary *>(e);
	if (u) {
		out << "<unary id=\"" << (int)e
		    << "\" op=\"" << operStrings[u->op]
		    << "\">\n";
		out << "<subexp1>\n";
		persistToXML(out, u->subExp1);
		out << "</subexp1>\n";
		out << "</unary>\n";
		return;
	}
	std::cerr << "unknown exp in persistToXML\n";
	assert(false);
}

void XMLProgParser::persistToXML(std::ostream &out, Cfg *cfg)
{
	out << "<cfg id=\"" << (int)cfg
	    << "\" wellformed=\"" << (int)cfg->m_bWellFormed
	    << "\" lastLabel=\"" << cfg->lastLabel
	    << "\" entryBB=\"" << (int)cfg->entryBB
	    << "\" exitBB=\"" << (int)cfg->exitBB
	    << "\">\n";

	for (std::list<PBB>::iterator it = cfg->m_listBB.begin(); it != cfg->m_listBB.end(); it++)
		persistToXML(out, *it);

	for (unsigned i = 0; i < cfg->Ordering.size(); i++)
		out << "<order bb=\"" << (int)cfg->Ordering[i] << "\"/>\n";

	for (unsigned i = 0; i < cfg->revOrdering.size(); i++)
		out << "<revorder bb=\"" << (int)cfg->revOrdering[i] << "\"/>\n";

	// TODO
	// MAPBB m_mapBB;
	// std::set<CallStatement *> callSites;
	// std::vector<PBB> BBs;        // Pointers to BBs from indices
	// std::map<PBB, int> indices;  // Indices from pointers to BBs
	// more
	out << "</cfg>\n";
}

void XMLProgParser::persistToXML(std::ostream &out, BasicBlock *bb)
{
	out << "<bb id=\"" << (int)bb
	    << "\" nodeType=\"" << bb->m_nodeType
	    << "\" labelNum=\"" << bb->m_iLabelNum
	    << "\" label=\"" << bb->m_labelStr
	    << "\" labelneeded=\"" << (int)bb->m_labelneeded
	    << "\" incomplete=\"" << (int)bb->m_bIncomplete
	    << "\" jumpreqd=\"" << (int)bb->m_bJumpReqd
	    << "\" m_traversed=\"" << bb->m_iTraversed
	    << "\" DFTfirst=\"" << bb->m_DFTfirst
	    << "\" DFTlast=\"" << bb->m_DFTlast
	    << "\" DFTrevfirst=\"" << bb->m_DFTrevfirst
	    << "\" DFTrevlast=\"" << bb->m_DFTrevlast
	    << "\" structType=\"" << bb->m_structType
	    << "\" loopCondType=\"" << bb->m_loopCondType;
	if (bb->m_loopHead)
		out << "\" m_loopHead=\"" << (int)bb->m_loopHead;
	if (bb->m_caseHead)
		out << "\" m_caseHead=\"" << (int)bb->m_caseHead;
	if (bb->m_condFollow)
		out << "\" m_condFollow=\"" << (int)bb->m_condFollow;
	if (bb->m_loopFollow)
		out << "\" m_loopFollow=\"" << (int)bb->m_loopFollow;
	if (bb->m_latchNode)
		out << "\" m_latchNode=\"" << (int)bb->m_latchNode;
	out << "\" ord=\"" << bb->ord
	    << "\" revOrd=\"" << bb->revOrd
	    << "\" inEdgesVisited=\"" << bb->inEdgesVisited
	    << "\" numForwardInEdges=\"" << bb->numForwardInEdges
	    << "\" loopStamp1=\"" << bb->loopStamps[0]
	    << "\" loopStamp2=\"" << bb->loopStamps[1]
	    << "\" revLoopStamp1=\"" << bb->revLoopStamps[0]
	    << "\" revLoopStamp2=\"" << bb->revLoopStamps[1]
	    << "\" traversed=\"" << (int)bb->traversed
	    << "\" hllLabel=\"" << (int)bb->hllLabel;
	if (bb->labelStr)
		out << "\" labelStr=\"" << bb->labelStr;
	out << "\" indentLevel=\"" << bb->indentLevel;
	// note the rediculous duplication here
	if (bb->immPDom)
		out << "\" immPDom=\"" << (int)bb->immPDom;
	if (bb->loopHead)
		out << "\" loopHead=\"" << (int)bb->loopHead;
	if (bb->caseHead)
		out << "\" caseHead=\"" << (int)bb->caseHead;
	if (bb->condFollow)
		out << "\" condFollow=\"" << (int)bb->condFollow;
	if (bb->loopFollow)
		out << "\" loopFollow=\"" << (int)bb->loopFollow;
	if (bb->latchNode)
		out << "\" latchNode=\"" << (int)bb->latchNode;
	out << "\" sType=\"" << (int)bb->sType
	    << "\" usType=\"" << (int)bb->usType
	    << "\" lType=\"" << (int)bb->lType
	    << "\" cType=\"" << (int)bb->cType
	    << "\">\n";

	for (unsigned i = 0; i < bb->m_InEdges.size(); i++)
		out << "<inedge bb=\"" << (int)bb->m_InEdges[i] << "\"/>\n";
	for (unsigned i = 0; i < bb->m_OutEdges.size(); i++)
		out << "<outedge bb=\"" << (int)bb->m_OutEdges[i] << "\"/>\n";

	LocationSet::iterator it;
	for (it = bb->liveIn.begin(); it != bb->liveIn.end(); it++) {
		out << "<livein>\n";
		persistToXML(out, *it);
		out << "</livein>\n";
	}

	if (bb->m_pRtls) {
		for (std::list<RTL *>::iterator it = bb->m_pRtls->begin(); it != bb->m_pRtls->end(); it++)
			persistToXML(out, *it);
	}
	out << "</bb>\n";
}

void XMLProgParser::persistToXML(std::ostream &out, RTL *rtl)
{
	out << "<rtl id=\"" << (int)rtl
	    << "\" addr=\"" << (int)rtl->nativeAddr
	    << "\">\n";
	for (std::list<Statement *>::iterator it = rtl->stmtList.begin(); it != rtl->stmtList.end(); it++) {
		out << "<stmt>\n";
		persistToXML(out, *it);
		out << "</stmt>\n";
	}
	out << "</rtl>\n";
}

void XMLProgParser::persistToXML(std::ostream &out, Statement *stmt)
{
	BoolAssign *b = dynamic_cast<BoolAssign *>(stmt);
	if (b) {
		out << "<boolasgn id=\"" << (int)stmt
		    << "\" number=\"" << b->number;
		if (b->parent)
			out << "\" parent=\"" << (int)b->parent;
		if (b->proc)
			out << "\" proc=\"" << (int)b->proc;
		out << "\" jtcond=\"" << b->jtCond
		    << "\" float=\"" << (int)b->bFloat
		    << "\" size=\"" << b->size
		    << "\">\n";
		if (b->pCond) {
			out << "<cond>\n";
			persistToXML(out, b->pCond);
			out << "</cond>\n";
		}
		out << "</boolasgn>\n";
		return;
	}
	ReturnStatement *r = dynamic_cast<ReturnStatement *>(stmt);
	if (r) {
		out << "<returnstmt id=\"" << (int)stmt
		    << "\" number=\"" << r->number;
		if (r->parent)
			out << "\" parent=\"" << (int)r->parent;
		if (r->proc)
			out << "\" proc=\"" << (int)r->proc;
		out << "\" retAddr=\"" << (int)r->retAddr
		    << "\">\n";

		ReturnStatement::iterator rr;
		for (rr = r->modifieds.begin(); rr != r->modifieds.end(); ++rr) {
			out << "<modifieds>\n";
			persistToXML(out, *rr);
			out << "</modifieds>\n";
		}
		for (rr = r->returns.begin(); rr != r->returns.end(); ++rr) {
			out << "<returns>\n";
			persistToXML(out, *rr);
			out << "</returns>\n";
		}

		out << "</returnstmt>\n";
		return;
	}
	CallStatement *c = dynamic_cast<CallStatement *>(stmt);
	if (c) {
		out << "<callstmt id=\"" << (int)stmt
		    << "\" number=\"" << c->number
		    << "\" computed=\"" << (int)c->m_isComputed;
		if (c->parent)
			out << "\" parent=\"" << (int)c->parent;
		if (c->proc)
			out << "\" proc=\"" << (int)c->proc;
		out << "\" returnAfterCall=\"" << (int)c->returnAfterCall
		    << "\">\n";

		if (c->pDest) {
			out << "<dest";
			if (c->procDest)
				out << " proc=\"" << (int)c->procDest << "\"";
			out << ">\n";
			persistToXML(out, c->pDest);
			out << "</dest>\n";
		}

		StatementList::iterator ss;
		for (ss = c->arguments.begin(); ss != c->arguments.end(); ++ss) {
			out << "<argument>\n";
			persistToXML(out, *ss);
			out << "</argument>\n";
		}

		for (ss = c->defines.begin(); ss != c->defines.end(); ++ss) {
			out << "<defines>\n";
			persistToXML(out, *ss);
			out << "</defines>\n";
		}

		out << "</callstmt>\n";
		return;
	}
	CaseStatement *ca = dynamic_cast<CaseStatement *>(stmt);
	if (ca) {
		out << "<casestmt id=\"" << (int)stmt
		    << "\" number=\"" << ca->number
		    << "\" computed=\"" << (int)ca->m_isComputed;
		if (ca->parent)
			out << "\" parent=\"" << (int)ca->parent;
		if (ca->proc)
			out << "\" proc=\"" << (int)ca->proc;
		out << "\">\n";
		if (ca->pDest) {
			out << "<dest>\n";
			persistToXML(out, ca->pDest);
			out << "</dest>\n";
		}
		// TODO
		// SWITCH_INFO *pSwitchInfo;   // Ptr to struct with info about the switch
		out << "</casestmt>\n";
		return;
	}
	BranchStatement *br = dynamic_cast<BranchStatement *>(stmt);
	if (br) {
		out << "<branchstmt id=\"" << (int)stmt
		    << "\" number=\"" << br->number
		    << "\" computed=\"" << (int)br->m_isComputed
		    << "\" jtcond=\"" << br->jtCond
		    << "\" float=\"" << (int)br->bFloat;
		if (br->parent)
			out << "\" parent=\"" << (int)br->parent;
		if (br->proc)
			out << "\" proc=\"" << (int)br->proc;
		out << "\">\n";
		if (br->pDest) {
			out << "<dest>\n";
			persistToXML(out, br->pDest);
			out << "</dest>\n";
		}
		if (br->pCond) {
			out << "<cond>\n";
			persistToXML(out, br->pCond);
			out << "</cond>\n";
		}
		out << "</branchstmt>\n";
		return;
	}
	GotoStatement *g = dynamic_cast<GotoStatement *>(stmt);
	if (g) {
		out << "<gotostmt id=\"" << (int)stmt
		    << "\" number=\"" << g->number
		    << "\" computed=\"" << (int) g->m_isComputed;
		if (g->parent)
			out << "\" parent=\"" << (int)g->parent;
		if (g->proc)
			out << "\" proc=\"" << (int)g->proc;
		out << "\">\n";
		if (g->pDest) {
			out << "<dest>\n";
			persistToXML(out, g->pDest);
			out << "</dest>\n";
		}
		out << "</gotostmt>\n";
		return;
	}
	PhiAssign *p = dynamic_cast<PhiAssign *>(stmt);
	if (p) {
		out << "<phiassign id=\"" << (int)stmt
		    << "\" number=\"" << p->number;
		if (p->parent)
			out << "\" parent=\"" << (int)p->parent;
		if (p->proc)
			out << "\" proc=\"" << (int)p->proc;
		out << "\">\n";
		out << "<lhs>\n";
		persistToXML(out, p->lhs);
		out << "</lhs>\n";
		PhiAssign::iterator it;
		for (it = p->begin(); it != p->end(); p++)
			out << "<def stmt=\"" << (int)it->def
			    << "\" exp=\"" << (int)it->e
			    << "\"/>\n";
		out << "</phiassign>\n";
		return;
	}
	Assign *a = dynamic_cast<Assign *>(stmt);
	if (a) {
		out << "<assign id=\"" << (int)stmt
		    << "\" number=\"" << a->number;
		if (a->parent)
			out << "\" parent=\"" << (int)a->parent;
		if (a->proc)
			out << "\" proc=\"" << (int)a->proc;
		out << "\">\n";
		out << "<lhs>\n";
		persistToXML(out, a->lhs);
		out << "</lhs>\n";
		out << "<rhs>\n";
		persistToXML(out, a->rhs);
		out << "</rhs>\n";
		if (a->type) {
			out << "<type>\n";
			persistToXML(out, a->type);
			out << "</type>\n";
		}
		if (a->guard) {
			out << "<guard>\n";
			persistToXML(out, a->guard);
			out << "</guard>\n";
		}
		out << "</assign>\n";
		return;
	}
	std::cerr << "unknown stmt in persistToXML\n";
	assert(false);
}
