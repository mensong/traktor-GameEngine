#ifndef traktor_flash_ActionFunction_H
#define traktor_flash_ActionFunction_H

#include "Flash/Action/ActionObject.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

class ActionVM;
class ActionFrame;
class ActionObject;
class ActionValue;

/*! \brief ActionScript callable function base.
 * \ingroup Flash
 */
class T_DLLCLASS ActionFunction : public ActionObject
{
	T_RTTI_CLASS(ActionFunction)

public:
	ActionFunction(const std::wstring& name);

	virtual ActionValue call(ActionVM* vm, ActionFrame* callerFrame, ActionObject* self) = 0;

	virtual std::wstring toString() const;

	void setName(const std::wstring& name) { m_name = name; }

	const std::wstring& getName() const { return m_name; }

private:
	std::wstring m_name;
};

	}
}

#endif	// traktor_flash_ActionFunction_H
