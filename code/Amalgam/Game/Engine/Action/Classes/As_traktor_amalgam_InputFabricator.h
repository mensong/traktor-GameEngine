#ifndef traktor_amalgam_As_traktor_amalgam_InputFabricator_H
#define traktor_amalgam_As_traktor_amalgam_InputFabricator_H

#include "Amalgam/Game/IEnvironment.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/ActionClass.h"

namespace traktor
{
	namespace amalgam
	{

/*! \brief Traktor input fabricator ActionScript wrapper.
 * \ingroup Amalgam
 */
class As_traktor_amalgam_InputFabricator : public flash::ActionClass
{
	T_RTTI_CLASS;

public:
	As_traktor_amalgam_InputFabricator(flash::ActionContext* context, IEnvironment* environment);

	virtual void initialize(flash::ActionObject* self);

	virtual void construct(flash::ActionObject* self, const flash::ActionValueArray& args);

	virtual flash::ActionValue xplicit(const flash::ActionValueArray& args);

private:
	IEnvironment* m_environment;

	void InputFabricator_fabricateSource(flash::CallArgs& ca);
};

	}
}

#endif	// traktor_amalgam_As_traktor_amalgam_InputFabricator_H
