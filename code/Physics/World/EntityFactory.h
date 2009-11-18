#ifndef traktor_physics_EntityFactory_H
#define traktor_physics_EntityFactory_H

#include "World/Entity/IEntityFactory.h"

#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

class PhysicsManager;

/*! \brief Physics entity factory.
 * \ingroup Physics
 */
class T_DLLCLASS EntityFactory : public world::IEntityFactory
{
	T_RTTI_CLASS;

public:
	EntityFactory(physics::PhysicsManager* physicsManager);

	virtual const TypeInfoSet getEntityTypes() const;

	virtual Ref< world::Entity > createEntity(
		world::IEntityBuilder* builder,
		const std::wstring& name,
		const world::EntityData& entityData,
		const Object* instanceData
	) const;

private:
	Ref< physics::PhysicsManager > m_physicsManager;
};

	}
}

#endif	// traktor_physics_EntityFactory_H
