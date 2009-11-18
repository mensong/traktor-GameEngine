#include "Animation/PathEntity/PathEntityFactory.h"
#include "Animation/PathEntity/PathEntityData.h"
#include "Animation/PathEntity/PathEntity.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.PathEntityFactory", PathEntityFactory, world::IEntityFactory)

const TypeInfoSet PathEntityFactory::getEntityTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< PathEntityData >());
	return typeSet;
}

Ref< world::Entity > PathEntityFactory::createEntity(world::IEntityBuilder* builder, const std::wstring& name, const world::EntityData& entityData, const Object* instanceData) const
{
	const PathEntityData* pathEntityData = checked_type_cast< const PathEntityData* >(&entityData);
	return pathEntityData->createEntity(builder);
}

	}
}
