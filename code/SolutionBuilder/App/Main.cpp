#include <set>
#include "Core/Io/Path.h"
#include "Core/Log/Log.h"
#include "Core/Misc/CommandLine.h"
#include "Core/Misc/TString.h"
#include "Core/Timer/Timer.h"
#include "SolutionBuilder/Version.h"
#include "SolutionBuilder/Aggregation.h"
#include "SolutionBuilder/ExternalDependency.h"
#include "SolutionBuilder/Project.h"
#include "SolutionBuilder/ProjectDependency.h"
#include "SolutionBuilder/Solution.h"
#include "SolutionBuilder/SolutionLoader.h"
#include "SolutionBuilder/CBlocks/SolutionBuilderCBlocks.h"
#include "SolutionBuilder/Dependencies/SolutionBuilderDependencies.h"
#include "SolutionBuilder/Eclipse/SolutionBuilderEclipse.h"
#include "SolutionBuilder/FBuild/SolutionBuilderFBuild.h"
#include "SolutionBuilder/GraphViz/SolutionBuilderGraphViz.h"
#include "SolutionBuilder/Make/SolutionBuilderMake2.h"
#include "SolutionBuilder/Msvc/SolutionBuilderMsvc.h"
#include "SolutionBuilder/Ninja/SolutionBuilderNinja.h"
#include "SolutionBuilder/Xcode/SolutionBuilderXcode.h"

using namespace traktor;
using namespace traktor::sb;

#define ERROR_UNKNOWN_FORMAT 1
#define ERROR_UNABLE_TO_READ_SOLUTION 2
#define ERROR_UNABLE_TO_RESOLVE_DEPENDENCIES 3
#define ERROR_UNABLE_TO_CREATE_BUILDER 4
#define ERROR_UNABLE_TO_CREATE_SOLUTION 5

void flattenIncludePaths(Project* project, std::map< std::wstring, std::set< std::wstring > >& outConfigurationIncludePaths)
{
	for (auto configuration : project->getConfigurations())
	{
		const auto& includePaths = configuration->getIncludePaths();
		outConfigurationIncludePaths[configuration->getName()].insert(
			includePaths.begin(),
			includePaths.end()
		);
	}

	for (auto dependency : project->getDependencies())
	{
		if (!dependency->getInheritIncludePaths())
			continue;

		if (auto projectDependency = dynamic_type_cast< const ProjectDependency* >(dependency))
			flattenIncludePaths(projectDependency->getProject(), outConfigurationIncludePaths);
		else if (auto externalDependency = dynamic_type_cast< const ExternalDependency* >(dependency))
			flattenIncludePaths(externalDependency->getProject(), outConfigurationIncludePaths);
	}
}

int main(int argc, const char** argv)
{
	T_FORCE_LINK_REF(ProjectDependency)
	T_FORCE_LINK_REF(ExternalDependency)

	CommandLine cmdLine(argc, argv);
	Ref< SolutionBuilder > builder;

	if (cmdLine.hasOption('f', L"format"))
	{
		std::wstring ide = cmdLine.getOption('f', L"format").getString();
		if (ide == L"cblocks")
			builder = new SolutionBuilderCBlocks();
		else if (ide == L"eclipse")
			builder = new SolutionBuilderEclipse();
		else if (ide == L"fbuild")
			builder = new SolutionBuilderFBuild();
		else if (ide == L"graphviz")
			builder = new SolutionBuilderGraphViz();
		else if (ide == L"make2")
			builder = new SolutionBuilderMake2();
		else if (ide == L"msvc")
			builder = new SolutionBuilderMsvc();
		else if (ide == L"ninja")
			builder = new SolutionBuilderNinja();
		else if (ide == L"xcode")
			builder = new SolutionBuilderXcode();
		else if (ide == L"dependencies")
			builder = new SolutionBuilderDependencies();
		else
		{
			traktor::log::error << L"Unknown format \"" << ide << L"\"." << Endl;
			return ERROR_UNKNOWN_FORMAT;
		}
	}
	else
		builder = new SolutionBuilderMsvc();

	if (cmdLine.hasOption('?') || cmdLine.hasOption('h', L"help"))
	{
		traktor::log::info << SB_TITLE << Endl;
		traktor::log::info << L"Usage : " << Path(cmdLine.getFile()).getFileName() << L" -[options] [solution]" << Endl;
		traktor::log::info << L"\t-f,-format=[format]	[\"cblocks\", \"eclipse\", \"graphviz\", \"msvc\"*, \"make\", \"make2\", \"ninja\", \"xcode\", \"dependencies\"]" << Endl;
		traktor::log::info << L"\t-rootPath=Path		Override solution root path" << Endl;
		traktor::log::info << L"\t-v,verbose			Verbose" << Endl;
		if (builder)
			builder->showOptions();
		return 0;
	}

	Timer timer;
	Ref< Solution > solution;

	if (cmdLine.getCount() >= 1)
	{
		Path solutionPath = cmdLine.getString(0);
		SolutionLoader solutionLoader;

		if (cmdLine.hasOption('v', L"verbose"))
		{
			traktor::log::info << SB_TITLE << Endl;
			traktor::log::info << L"Loading solution \"" << solutionPath.getPathName() << L"\"..." << Endl;
		}

		solution = solutionLoader.load(cmdLine.getString(0));
		if (!solution)
		{
			traktor::log::error << L"Unable to read solution \"" << solutionPath.getPathName() << L"\"" << Endl;
			return ERROR_UNABLE_TO_READ_SOLUTION;
		}

		if (cmdLine.hasOption(L"rootPath"))
		{
			Path rootPath(cmdLine.getOption(L"rootPath").getString());
			solution->setRootPath(rootPath.normalized().getPathName());
		}

		if (cmdLine.hasOption('v', L"verbose"))
			traktor::log::info << L"Using root path \"" << solution->getRootPath() << L"\"" << Endl;

		if (cmdLine.hasOption('v', L"verbose"))
			traktor::log::info << L"Flatten include paths..." << Endl;

		for (auto project : solution->getProjects())
		{
			std::map< std::wstring, std::set< std::wstring > > configurationIncludePaths;
			flattenIncludePaths(project, configurationIncludePaths);

			for (auto configuration : project->getConfigurations())
			{
				const auto& includePaths = configurationIncludePaths[configuration->getName()];

				if (cmdLine.hasOption('v', L"verbose"))
				{
					traktor::log::info << L"Include paths of \"" << project->getName() << L"\" " << configuration->getName() << Endl;
					for (auto includePath : includePaths)
						traktor::log::info << L"\t" << includePath << Endl;
				}

				configuration->setIncludePaths(std::vector< std::wstring >(
					includePaths.begin(),
					includePaths.end()
				));
			}
		}
	}

	if (!builder || !builder->create(cmdLine))
	{
		traktor::log::error << L"Unable to create solution builder." << Endl;
		return ERROR_UNABLE_TO_CREATE_BUILDER;
	}

	if (cmdLine.hasOption('v', L"verbose"))
		traktor::log::info << L"Generating target solution..." << Endl;

	if (solution)
	{
		if (!builder->generate(solution))
		{
			traktor::log::error << L"Unable to generate target solution." << Endl;
			return ERROR_UNABLE_TO_CREATE_SOLUTION;
		}
	}

	if (cmdLine.hasOption('v', L"verbose"))
		traktor::log::info << L"Finished successfully in " << timer.getElapsedTime() << L" second(s)." << Endl;

	return 0;
}
