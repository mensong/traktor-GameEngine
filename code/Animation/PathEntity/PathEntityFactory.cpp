#include "Animation/PathEntity/PathEntityFactory.h"
#include "Animation/PathEntity/PathEntityData.h"
#include "Animation/PathEntity/PathEntity.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.PathEntityFactory", PathEntityFactory, world::EntityFactory)

const TypeSet PathEntityFactory::getEntityTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< PathEntityData >());
	return typeSet;
}

world::Entity* PathEntityFactory::createEntity(world::EntityBuilder* builder, const world::EntityData& entityData) const
{
	const PathEntityData* pathEntityData = checked_type_cast< const PathEntityData* >(&entityData);
	return pathEntityData->createEntity(builder);
}

	}
}
