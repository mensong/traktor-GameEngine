#ifndef traktor_terrain_MaterialMaskFactory_H
#define traktor_terrain_MaterialMaskFactory_H

#include "Resource/IResourceFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_TERRAIN_EXPORT)
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

	namespace terrain
	{

class T_DLLCLASS MaterialMaskFactory : public resource::IResourceFactory
{
	T_RTTI_CLASS;

public:
	MaterialMaskFactory(db::Database* db);

	virtual const TypeInfoSet getResourceTypes() const;

	virtual bool isCacheable() const;

	virtual Ref< Object > create(resource::IResourceManager* resourceManager, const TypeInfo& resourceType, const Guid& guid);

private:
	Ref< db::Database > m_db;
};

	}
}

#endif	// traktor_terrain_MaterialMaskFactory_H
