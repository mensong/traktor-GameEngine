#ifndef traktor_world_EntityManager_H
#define traktor_world_EntityManager_H

#include <map>
#include "World/Entity/IEntityManager.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

/*! \brief Entity manager.
 * \ingroup World
 *
 * Entity manager is a class to help
 * accessing entities based on various
 * queries.
 */
class T_DLLCLASS EntityManager : public IEntityManager
{
	T_RTTI_CLASS;

public:
	virtual void addEntity(Entity* entity);

	virtual void insertEntity(Entity* entity);

	virtual void removeEntity(Entity* entity);

	virtual uint32_t getEntitiesOf(const TypeInfo& entityType, RefArray< Entity >& outEntities) const;

	virtual uint32_t getEntityCount(const TypeInfo& entityType) const;

	virtual Ref< Entity > getEntity(const TypeInfo& entityType, uint32_t index) const;

	virtual const RefArray< Entity >& getEntities() const;

private:
	struct Range
	{
		uint32_t start;
		uint32_t count;
	};

	RefArray< Entity > m_entities;
	std::map< const TypeInfo*, Range > m_typeRanges;
};

	}
}

#endif	// traktor_world_EntityManager_H
