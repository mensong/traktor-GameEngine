#include <Core/Serialization/ISerializer.h>
#include <Core/Serialization/Member.h>
#include <Core/Serialization/MemberStl.h>
#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvcLibrarianTool.h"
#include "SolutionBuilderLIB/Msvc/GeneratorContext.h"
#include "SolutionBuilderLIB/Project.h"
#include "SolutionBuilderLIB/Configuration.h"

using namespace traktor;

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"SolutionBuilderMsvcLibrarianTool", 0, SolutionBuilderMsvcLibrarianTool, SolutionBuilderMsvcTool)

bool SolutionBuilderMsvcLibrarianTool::generate(GeneratorContext& context, Solution* solution, Project* project, Configuration* configuration, traktor::OutputStream& os) const
{
	os << L"<Tool" << Endl;
	os << IncreaseIndent;
	os << L"Name=\"VCLibrarianTool\"" << Endl;

	// Output file.
	if (configuration->getTargetProfile() == Configuration::TpDebug)
		os << L"OutputFile=\"$(OutDir)/" << project->getName() << L"_d.lib\"" << Endl;
	else
		os << L"OutputFile=\"$(OutDir)/" << project->getName() << L".lib\"" << Endl;

	// Static options.
	for (std::map< std::wstring, std::wstring >::const_iterator i = m_staticOptions.begin(); i != m_staticOptions.end(); ++i)
		os << i->first << L"=\"" << context.format(i->second) << L"\"" << Endl;

	os << DecreaseIndent;
	os << L"/>" << Endl;
	return true;
}

bool SolutionBuilderMsvcLibrarianTool::serialize(traktor::ISerializer& s)
{
	s >> MemberStlMap< std::wstring, std::wstring >(L"staticOptions", m_staticOptions);
	return true;
}
