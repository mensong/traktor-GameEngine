#ifndef Condition_H
#define Condition_H

#include <vector>
#include <string>
#include <Core/Object.h>
#include <Core/Io/OutputStream.h>

class Source;

class Condition : public traktor::Object
{
	T_RTTI_CLASS(Condition)

public:
	virtual void check(const Source& source, bool isHeader, traktor::OutputStream& report) const = 0;
};

#endif	// Condition_H
