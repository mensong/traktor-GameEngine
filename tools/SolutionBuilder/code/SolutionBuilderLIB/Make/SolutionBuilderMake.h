#ifndef SolutionBuilderMake_H
#define SolutionBuilderMake_H

#include <set>
#include <string>
#include <Core/Misc/CommandLine.h>
#include "SolutionBuilderLIB/SolutionBuilder.h"

namespace traktor
{

class StreamAdapter;

}

class Project;
class Configuration;
class DependencyCache;

class SolutionBuilderMake : public SolutionBuilder
{
	T_RTTI_CLASS(SolutionBuilderMake)

public:
	enum Dialect
	{
		MdNMake,	// Microsoft NMake
		MdGnuMake	// GNU make
	};
	
	enum Platform
	{
		MpWin32,
		MpMacOSX,
		MpLinux
	};

	SolutionBuilderMake();
	
	virtual bool create(const traktor::CommandLine& cmdLine);
	
	virtual bool generate(Solution* solution);

	virtual void showOptions() const;

private:
	Dialect m_dialect;
	Platform m_platform;
	std::wstring m_config;
	traktor::Ref< DependencyCache > m_dependencyCache;

	bool generateProject(Solution* solution, Project* project);

	void collectLinkDependencies(
		Solution* buildSolution,
		Project* project,
		const std::wstring& configurationName,
		std::set< std::wstring >& outLibraryPaths,
		std::vector< std::wstring >& outLibraryNames
	);

	bool scanDependencies(
		Solution* solution,
		Configuration* configuration,
		const std::wstring& fileName,
		std::set< std::wstring >& outDependencies
	);

	bool scanDependencies(
		Solution* solution,
		Configuration* configuration,
		const std::wstring& fileName,
		std::set< std::wstring >& visitedDependencies,
		std::set< std::wstring >& resolvedDependencies
	);
};

#endif	// SolutionBuilderMake_H
