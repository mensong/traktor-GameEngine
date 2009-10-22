#ifndef ExternalDependency_H
#define ExternalDependency_H

#include "Dependency.h"

class Solution;
class Project;

class ExternalDependency : public Dependency
{
	T_RTTI_CLASS(ExternalDependency)

public:
	ExternalDependency(const std::wstring& solutionFileName = L"", const std::wstring& projectName = L"");

	void setSolutionFileName(const std::wstring& solutionFileName);

	const std::wstring& getSolutionFileName() const;

	Solution* getSolution() const;

	Project* getProject() const;

	virtual std::wstring getName() const;

	virtual std::wstring getLocation() const;

	virtual bool resolve(SolutionLoader* solutionLoader);

	virtual int getVersion() const;

	virtual bool serialize(traktor::Serializer& s);

private:
	std::wstring m_solutionFileName;
	std::wstring m_projectName;
	traktor::Ref< Solution > m_solution;
	traktor::Ref< Project > m_project;
	traktor::RefList< ExternalDependency > m_dependencies;
};

#endif	// ExternalDependency_H
