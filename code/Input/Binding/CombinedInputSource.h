#ifndef traktor_input_CombinedInputSource_H
#define traktor_input_CombinedInputSource_H

#include <list>
#include "Core/RefArray.h"
#include "Input/Binding/IInputSource.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace input
	{

class T_DLLCLASS CombinedInputSource : public IInputSource
{
	T_RTTI_CLASS;

public:
	CombinedInputSource(const RefArray< IInputSource >& sources);

	virtual std::wstring getDescription() const;
	
	virtual float read(InputSystem* inputSystem, float T, float dT);
	
private:
	RefArray< IInputSource > m_sources;
};

	}
}

#endif	// traktor_input_CombinedInputSource_H
