#ifndef traktor_world_IEntityManager_H
#define traktor_world_IEntityManager_H

#include "Core/Object.h"
#include "Core/RefArray.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class Entity;

/*! \brief Entity manager interface.
 * \ingroup World
 *
 * Entity manager is a class to help
 * accessing entities based on various
 * queries.
 */
class T_DLLCLASS IEntityManager : public Object
{
	T_RTTI_CLASS;

public:
	/*! \brief Add entity to manager.
	 *
	 * \param name Instance name.
	 * \param entity Entity to add.
	 */
	virtual void addEntity(const std::wstring& name, Entity* entity) = 0;

	/*! \brief Insert entity in manager.
	 *
	 * This is method checks to ensure entity isn't
	 * added multiple times.
	 *
	 * \param name Instance name.
	 * \param entity Entity to insert.
	 */
	virtual void insertEntity(const std::wstring& name, Entity* entity) = 0;

	/*! \brief Remove entity from manager.
	 *
	 * \param entity Entity to remove.
	 */
	virtual void removeEntity(Entity* entity) = 0;

	/*! \brief Get named entity.
	 *
	 * \param name Name of entity.
	 * \return Found entity.
	 */
	virtual Entity* getEntity(const std::wstring& name) const = 0;

	/*! \brief Get named entities.
	 *
	 * \param name Name of entities.
	 * \param outEntities Array of found entities.
	 * \return Number of entities found.
	 */
	virtual uint32_t getEntities(const std::wstring& name, RefArray< Entity >& outEntities) const = 0;

	/*! \brief Get all entities of certain type.
	 *
	 * \param entityType Type of entity.
	 * \param outEntities Array of found entities.
	 * \return Number of entities found.
	 */
	virtual uint32_t getEntitiesOf(const TypeInfo& entityType, RefArray< Entity >& outEntities) const = 0;

	/*! \brief Get number of entities which are a certain type.
	 *
	 * \param entityType Type of entity.
	 * \return Number of entities of type.
	 */
	virtual uint32_t getEntityCount(const TypeInfo& entityType) const = 0;

	/*! \brief Get entity of certain type.
	 *
	 * \param entityType Type of entity.
	 * \param index Index of entity of same type.
	 * \return Found entity, null if no entity found.
	 */
	virtual Entity* getEntity(const TypeInfo& entityType, uint32_t index) const = 0;

	/*! \brief Get named entity of certain type.
	 *
	 * \param entityType Type of entity.
	 * \param name Name of entity instance.
	 * \return Found entity, null if no entity found.
	 */
	virtual Entity* getEntity(const TypeInfo& entityType, const std::wstring& name) const = 0;

	/*! \brief Get all entities of certain type.
	 *
	 * \param EntityType Type of entity.
	 * \param outEntities Array of found entities.
	 * \return Number of entities found.
	 */
	template < typename EntityType >
	int getEntitiesOf(RefArray< EntityType >& outEntities) const
	{
		return getEntitiesOf(
			type_of< EntityType >(),
			*reinterpret_cast< RefArray< Entity >* >(&outEntities)
		);
	}

	/*! \brief Get number of entities of certain type.
	 *
	 * \param EntityType Type of entity.
	 * \return Number of entities of type.
	 */
	template < typename EntityType >
	uint32_t getEntityCount() const
	{
		return getEntityCount(type_of< EntityType >());
	}

	/*! \brief Get first entity of a certain type.
	 *
	 * \param EntityType Type of entity.
	 * \param index Index of entity of same type.
	 * \return Found entity, null if no entity found.
	 */
	template < typename EntityType >
	EntityType* getEntity(uint32_t index) const
	{
		Entity* entity = getEntity(type_of< EntityType >(), index);
		return static_cast< EntityType* >(entity);
	}

	/*! \brief Get named entity of certain type.
	 *
	 * \param entityType Type of entity.
	 * \param name Name of entity instance.
	 * \return Found entity, null if no entity found.
	 */
	template < typename EntityType >
	EntityType* getEntity(const std::wstring& name) const
	{
		Entity* entity = getEntity(type_of< EntityType >(), name);
		return static_cast< EntityType* >(entity);
	}
};

	}
}

#endif	// traktor_world_IEntityManager_H
