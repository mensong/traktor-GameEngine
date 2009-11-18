#include "Render/Sw/ProgramResourceSw.h"
#include "Render/ShaderGraph.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramResourceSw", ProgramResourceSw, ProgramResource)

ProgramResourceSw::ProgramResourceSw(const ShaderGraph* shaderGraph)
:	m_shaderGraph(const_cast< ShaderGraph* >(shaderGraph))
{
}

bool ProgramResourceSw::serialize(ISerializer& s)
{
	if (!ProgramResource::serialize(s))
		return false;

	return s >> MemberRef< ShaderGraph >(L"shaderGraph", m_shaderGraph);
}

	}
}
