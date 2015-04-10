/*
 * Copyright (C) 2004, Trent Waddington
 */
/*==============================================================================
 * FILE:       generic.h
 * OVERVIEW:   Provides the definition for the generic exp tranformer.
 *============================================================================*/

#ifndef GENERIC_EXP_TRANSFORMER_H
#define GENERIC_EXP_TRANSFORMER_H

class GenericExpTransformer : public ExpTransformer
{
protected:
	Exp *match, *where, *become;

	bool checkCond(Exp *cond, Exp *bindings);
	Exp *applyFuncs(Exp *rhs);
public:
	GenericExpTransformer(Exp *match, Exp *where, Exp *become) : match(match), where(where), become(become) {}
	virtual Exp *applyTo(Exp *e, bool &bMod);
};

#endif
