#ifndef ProjectDependency_H
#define ProjectDependency_H

#include "Dependency.h"

class Project;

class ProjectDependency : public Dependency
{
	T_RTTI_CLASS;

public:
	ProjectDependency(Project* project = 0);

	void setProject(Project* project);

	Project* getProject() const;

	virtual std::wstring getName() const;

	virtual std::wstring getLocation() const;

	virtual bool resolve(SolutionLoader* solutionLoader);

	virtual bool serialize(traktor::ISerializer& s);

private:
	traktor::Ref< Project > m_project;
};

#endif	// ProjectDependency_H
