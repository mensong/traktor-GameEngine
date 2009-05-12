#include "Physics/Editor/RigidEntityPipeline.h"
#include "Physics/World/RigidEntityData.h"
#include "Physics/BodyDesc.h"
#include "Physics/MeshShapeDesc.h"
#include "Editor/PipelineManager.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.physics.RigidEntityPipeline", RigidEntityPipeline, world::EntityPipeline)

TypeSet RigidEntityPipeline::getAssetTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< RigidEntityData >());
	return typeSet;
}

bool RigidEntityPipeline::buildDependencies(
	editor::PipelineManager* pipelineManager,
	const Object* sourceAsset,
	Ref< const Object >& outBuildParams
) const
{
	if (const RigidEntityData* rigidEntityData = dynamic_type_cast< const RigidEntityData* >(sourceAsset))
	{
		pipelineManager->addDependency(rigidEntityData->getBodyDesc());

		const world::SpatialEntityData* entityData = rigidEntityData->getEntityData();
		if (entityData)
			pipelineManager->addDependency(entityData);
	}
	return true;
}

	}
}
