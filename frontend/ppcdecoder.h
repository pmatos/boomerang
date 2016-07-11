/**
 * \file
 * \brief The implementation of the instruction decoder for PPC.
 *
 * \authors
 * Copyright (C) 1996-2001, The University of Queensland
 * \authors
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * \copyright
 * See the file "LICENSE.TERMS" for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */

#ifndef PPCDECODER
#define PPCDECODER

class Prog;
class NJMCDecoder;
struct DecodeResult;

class PPCDecoder : public NJMCDecoder {
public:
	PPCDecoder(Prog *prog);

	/*
	 * Decodes the machine instruction at pc and returns an RTL instance for
	 * the instruction.
	 */
	virtual DecodeResult &decodeInstruction(ADDRESS pc, int delta);

	/*
	 * Disassembles the machine instruction at pc and returns the number of
	 * bytes disassembled. Assembler output goes to global _assembly
	 */
	virtual int decodeAssemblyInstruction(ADDRESS pc, int delta);

private:
	/*
	 * Various functions to decode the operands of an instruction into an Exp* representation.
	 */
	Exp *dis_Eaddr(ADDRESS pc, int size = 0);
	Exp *dis_RegImm(ADDRESS pc);
	Exp *dis_Reg(unsigned r);
	Exp *dis_RAmbz(unsigned r);  // Special for rA of certain instructions

	void unused(int x);
	RTL *createBranchRtl(ADDRESS pc, std::list<Statement *> *stmts, const char *name);
	bool isFuncPrologue(ADDRESS hostPC);
	DWord getDword(ADDRESS lc);
};

#endif
