#ifndef SingleEmptyLine_H
#define SingleEmptyLine_H

#include "Condition.h"

class SingleEmptyLine : public Condition
{
	T_RTTI_CLASS(SingleEmptyLine)

public:
	virtual void check(const Source& source, bool isHeader, traktor::OutputStream& report) const;
};

#endif	// SingleEmptyLine_H
