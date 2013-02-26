#include "Physics/World/ArticulatedEntity.h"
#include "Physics/World/ArticulatedEntityData.h"
#include "Physics/World/EntityFactory.h"
#include "Physics/World/RigidEntity.h"
#include "Physics/World/RigidEntityData.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.EntityFactory", EntityFactory, world::IEntityFactory)

EntityFactory::EntityFactory(
	world::IEntityEventManager* eventManager,
	resource::IResourceManager* resourceManager,
	PhysicsManager* physicsManager
)
:	m_eventManager(eventManager)
,	m_resourceManager(resourceManager)
,	m_physicsManager(physicsManager)
{
}

const TypeInfoSet EntityFactory::getEntityTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< RigidEntityData >());
	typeSet.insert(&type_of< ArticulatedEntityData >());
	return typeSet;
}

Ref< world::Entity > EntityFactory::createEntity(
	const world::IEntityBuilder* builder,
	const world::EntityData& entityData
) const
{
	if (const RigidEntityData* rigidEntityData = dynamic_type_cast< const RigidEntityData* >(&entityData))
		return rigidEntityData->createEntity(builder, m_eventManager, m_resourceManager, m_physicsManager);

	if (const ArticulatedEntityData* articulatedEntityData = dynamic_type_cast< const ArticulatedEntityData* >(&entityData))
		return articulatedEntityData->createEntity(builder, m_physicsManager);

	return 0;
}

	}
}
