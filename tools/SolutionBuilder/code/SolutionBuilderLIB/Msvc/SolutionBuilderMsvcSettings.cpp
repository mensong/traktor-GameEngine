#include <Core/Serialization/ISerializer.h>
#include <Core/Serialization/Member.h>
#include <Core/Serialization/MemberRef.h>
#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvcSettings.h"
#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvcProject.h"

using namespace traktor;

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"SolutionBuilderMsvcSettings", 0, SolutionBuilderMsvcSettings, ISerializable)

bool SolutionBuilderMsvcSettings::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"slnVersion", m_slnVersion);
	s >> Member< std::wstring >(L"vsVersion", m_vsVersion);
	s >> MemberRef< SolutionBuilderMsvcProject >(L"project", m_project);
	return true;
}
