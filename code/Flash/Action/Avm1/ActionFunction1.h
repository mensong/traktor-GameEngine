#ifndef traktor_flash_ActionFunction1_H
#define traktor_flash_ActionFunction1_H

#include "Flash/Action/ActionFunction.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

class ActionDictionary;

/*! \brief ActionScript script function.
 * \ingroup Flash
 */
class T_DLLCLASS ActionFunction1 : public ActionFunction
{
	T_RTTI_CLASS;

public:
	ActionFunction1(
		ActionContext* context,
		const char* name,
		const IActionVMImage* image,
		uint16_t argumentCount,
		const std::vector< std::string >& argumentsIntoVariables,
		const SmallMap< uint32_t, ActionValue >& variables,
		const ActionDictionary* dictionary
	);

	virtual ActionValue call(ActionObject* self, ActionObject* super, const ActionValueArray& args);

protected:
	virtual void trace(visitor_t visitor) const;

	virtual void dereference();

private:
	Ref< const IActionVMImage > m_image;
	uint16_t m_argumentCount;
	std::vector< uint32_t > m_argumentsIntoVariables;
	SmallMap< uint32_t, ActionValue > m_variables;
	Ref< const ActionDictionary > m_dictionary;
};

	}
}

#endif	// traktor_flash_ActionFunction1_H
