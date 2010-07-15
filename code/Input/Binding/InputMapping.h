#ifndef traktor_input_InputMapping_H
#define traktor_input_InputMapping_H

#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Input/Binding/InputValueSet.h"

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

class InputMappingData;
class InputState;
class InputSystem;
class InputValueSource;

/*! \brief Input sources to state mapping.
 * \ingroup Input
 *
 * Each source is mapped into a linear array
 * of values (each value tagged with an unique, literal, id)
 * and are then consumed by defined states.
 *
 * Each state may have a chain of input nodes in order
 * to modify the source values into the final state's value.
 */
class T_DLLCLASS InputMapping : public Object
{
	T_RTTI_CLASS;
	
public:
	bool create(const InputMappingData* data);

	void update(InputSystem* inputSystem);
	
	InputState* get(const std::wstring& id) const;
	
private:
	RefArray< InputValueSource > m_sources;
	std::map< std::wstring, Ref< InputState > > m_states;
	InputValueSet m_valueSet;
};
	
	}
}

#endif	// traktor_input_InputMapping_H
