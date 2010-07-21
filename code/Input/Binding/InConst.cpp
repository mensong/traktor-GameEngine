#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Input/Binding/InConst.h"
#include "Input/Binding/InputValueSet.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.InConst", 0, InConst, IInputNode)

InConst::InConst()
{
}

InConst::InConst(float value)
:	m_value(value)
{
}

Ref< IInputNode::Instance > InConst::createInstance() const
{
	return 0;
}

float InConst::evaluate(
	Instance* instance,
	const InputValueSet& valueSet,
	float T,
	float dT
) const
{
	return m_value;
}

bool InConst::serialize(ISerializer& s)
{
	return s >> Member< float >(L"value", m_value);
}
	
	}
}
