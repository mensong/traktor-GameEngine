#ifndef traktor_input_CombinedInputSourceData_H
#define traktor_input_CombinedInputSourceData_H

#include "Input/InputTypes.h"
#include "Input/Binding/IInputSourceData.h"

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
	
class T_DLLCLASS CombinedInputSourceData : public IInputSourceData
{
	T_RTTI_CLASS;

public:
	CombinedInputSourceData();
	
	CombinedInputSourceData(const RefArray< IInputSourceData >& sources);

	void addSource(IInputSourceData* source);

	const RefArray< IInputSourceData >& getSources() const;
	
	virtual Ref< IInputSource > createInstance() const;

	virtual bool serialize(ISerializer& s);

private:
	RefArray< IInputSourceData > m_sources;
};

	}
}

#endif	// traktor_input_CombinedInputSourceData_H
