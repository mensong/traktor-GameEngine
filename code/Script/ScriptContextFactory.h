#ifndef traktor_script_ScriptContextFactory_H
#define traktor_script_ScriptContextFactory_H

#include "Core/Heap/Ref.h"
#include "Resource/ResourceFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCRIPT_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

class Database;

	}

	namespace script
	{

class ScriptManager;

class T_DLLCLASS ScriptContextFactory : public resource::ResourceFactory
{
	T_RTTI_CLASS(ScriptContextFactory)

public:
	ScriptContextFactory(db::Database* database, ScriptManager* scriptManager);

	virtual const TypeSet getResourceTypes() const;

	virtual Object* create(const Type& resourceType, const Guid& guid);

private:
	Ref< db::Database > m_database;
	Ref< ScriptManager > m_scriptManager;
};

	}
}

#endif	// traktor_script_ScriptContextFactory_H
