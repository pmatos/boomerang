/**
 * \file
 * \brief Provides the definition for the transformer and related classes.
 *
 * \authors
 * Copyright (C) 2004, Trent Waddington
 */

#ifndef TRANSFORMER_H
#define TRANSFORMER_H

#include <list>

class ExpTransformer {
protected:
	static std::list<ExpTransformer *> transformers;
public:
	ExpTransformer();
	virtual ~ExpTransformer() { }  // Prevent gcc4 warning

	static void loadAll();

	virtual Exp *applyTo(Exp *e, bool &bMod) = 0;
	static Exp *applyAllTo(Exp *e, bool &bMod);
};

#endif
