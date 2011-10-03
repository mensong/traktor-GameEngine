#include "Terrain/EntityFactory.h"
#include "Terrain/TerrainEntityData.h"
#include "Terrain/TerrainEntity.h"
#include "Terrain/OceanEntityData.h"
#include "Terrain/OceanEntity.h"
#include "Terrain/UndergrowthEntityData.h"
#include "Terrain/UndergrowthEntity.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.EntityFactory", EntityFactory, world::IEntityFactory)

EntityFactory::EntityFactory(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, bool editorMode)
:	m_resourceManager(resourceManager)
,	m_renderSystem(renderSystem)
,	m_editorMode(editorMode)
{
}

const TypeInfoSet EntityFactory::getEntityTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< TerrainEntityData >());
	typeSet.insert(&type_of< OceanEntityData >());
	typeSet.insert(&type_of< UndergrowthEntityData >());
	return typeSet;
}

Ref< world::Entity > EntityFactory::createEntity(world::IEntityBuilder* builder, const world::EntityData& entityData) const
{
	Ref< world::Entity > entity;

	if (is_a< TerrainEntityData >(&entityData))
	{
		Ref< TerrainEntity > terrainEntity = new TerrainEntity(m_renderSystem, m_editorMode);
		if (terrainEntity->create(m_resourceManager, static_cast< const TerrainEntityData& >(entityData)))
			return terrainEntity;
	}
	else if (is_a< OceanEntityData >(&entityData))
	{
		Ref< OceanEntity > oceanEntity = new OceanEntity();
		if (oceanEntity->create(m_resourceManager, m_renderSystem, static_cast< const OceanEntityData& >(entityData)))
			return oceanEntity;
	}
	else if (is_a< UndergrowthEntityData >(&entityData))
		entity = static_cast< const UndergrowthEntityData* >(&entityData)->createEntity(m_resourceManager, m_renderSystem);

	return entity;
}

	}
}
