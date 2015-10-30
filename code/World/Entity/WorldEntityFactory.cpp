#include "Render/ITexture.h"
#include "Render/Shader.h"
#include "Resource/IResourceManager.h"
#include "World/IEntityBuilder.h"
#include "World/IEntityComponentData.h"
#include "World/Entity/CameraEntity.h"
#include "World/Entity/CameraEntityData.h"
#include "World/Entity/ComponentEntity.h"
#include "World/Entity/ComponentEntityData.h"
#include "World/Entity/DecalEntity.h"
#include "World/Entity/DecalEntityData.h"
#include "World/Entity/DecalEvent.h"
#include "World/Entity/DecalEventData.h"
#include "World/Entity/DirectionalLightEntity.h"
#include "World/Entity/DirectionalLightEntityData.h"
#include "World/Entity/ExternalEntityData.h"
#include "World/Entity/GodRayEntity.h"
#include "World/Entity/GodRayEntityData.h"
#include "World/Entity/GroupEntity.h"
#include "World/Entity/GroupEntityData.h"
#include "World/Entity/NullEntity.h"
#include "World/Entity/NullEntityData.h"
#include "World/Entity/PointLightEntity.h"
#include "World/Entity/PointLightEntityData.h"
#include "World/Entity/ScriptComponent.h"
#include "World/Entity/ScriptComponentData.h"
#include "World/Entity/SpotLightEntity.h"
#include "World/Entity/SpotLightEntityData.h"
#include "World/Entity/SwitchEntity.h"
#include "World/Entity/SwitchEntityData.h"
#include "World/Entity/VolumeEntity.h"
#include "World/Entity/VolumeEntityData.h"
#include "World/Entity/WorldEntityFactory.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.WorldEntityFactory", WorldEntityFactory, IEntityFactory)

WorldEntityFactory::WorldEntityFactory(resource::IResourceManager* resourceManager, bool editor)
:	m_resourceManager(resourceManager)
,	m_editor(editor)
{
}

const TypeInfoSet WorldEntityFactory::getEntityTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< ExternalEntityData >());
	typeSet.insert(&type_of< GodRayEntityData >());
	typeSet.insert(&type_of< GroupEntityData >());
	typeSet.insert(&type_of< CameraEntityData >());
	typeSet.insert(&type_of< DecalEntityData >());
	typeSet.insert(&type_of< DirectionalLightEntityData >());
	typeSet.insert(&type_of< NullEntityData >());
	typeSet.insert(&type_of< PointLightEntityData >());
	typeSet.insert(&type_of< SpotLightEntityData >());
	typeSet.insert(&type_of< SwitchEntityData >());
	typeSet.insert(&type_of< VolumeEntityData >());
	typeSet.insert(&type_of< ComponentEntityData >());
	return typeSet;
}

const TypeInfoSet WorldEntityFactory::getEntityEventTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< DecalEventData >());
	return typeSet;
}

const TypeInfoSet WorldEntityFactory::getEntityComponentTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< ScriptComponentData >());
	return typeSet;
}

Ref< Entity > WorldEntityFactory::createEntity(const IEntityBuilder* builder, const EntityData& entityData) const
{
	if (const ExternalEntityData* externalEntityData = dynamic_type_cast< const ExternalEntityData* >(&entityData))
	{
		resource::Proxy< EntityData > resolvedEntityData;
		if (!m_resourceManager->bind(externalEntityData->getEntityData(), resolvedEntityData))
			return 0;

		resolvedEntityData->setName(externalEntityData->getName());
		resolvedEntityData->setTransform(externalEntityData->getTransform());

		if (m_editor)
		{
			// Due to scene editor collecting adapters we cannot continue to create
			// using the aggregated builder thus we have to manually create child entities.
			const IEntityFactory* factory = builder->getFactory(resolvedEntityData);
			if (factory)
				return factory->createEntity(builder->getCompositeEntityBuilder(), *resolvedEntityData.getResource());
			else
				return 0;
		}
		else
			return builder->create(resolvedEntityData.getResource());
	}

	if (const GodRayEntityData* godRayData = dynamic_type_cast< const GodRayEntityData* >(&entityData))
		return new GodRayEntity(godRayData->getTransform());

	if (const GroupEntityData* groupData = dynamic_type_cast< const GroupEntityData* >(&entityData))
	{
		Ref< GroupEntity > groupEntity = new GroupEntity(groupData->getTransform());

		const RefArray< EntityData >& groupChildEntityData = groupData->getEntityData();
		for (RefArray< EntityData >::const_iterator i = groupChildEntityData.begin(); i != groupChildEntityData.end(); ++i)
		{
			Ref< Entity > childEntity = builder->create(*i);
			if (childEntity)
				groupEntity->addEntity(childEntity);
		}

		return groupEntity;
	}

	if (const CameraEntityData* cameraData = dynamic_type_cast< const CameraEntityData* >(&entityData))
		return new CameraEntity(cameraData);

	if (const DecalEntityData* decalData = dynamic_type_cast< const DecalEntityData* >(&entityData))
	{
		resource::Proxy< render::Shader > shader;
		if (!m_resourceManager->bind(decalData->getShader(), shader))
			return 0;

		Ref< DecalEntity > decalEntity = new DecalEntity(
			decalData->getTransform(),
			decalData->getSize(),
			decalData->getThickness(),
			decalData->getAlpha(),
			decalData->getCullDistance(),
			shader
		);

		return decalEntity;
	}

	if (const DirectionalLightEntityData* directionalLightData = dynamic_type_cast< const DirectionalLightEntityData* >(&entityData))
	{
		resource::Proxy< render::ITexture > cloudShadowTexture;
		if (directionalLightData->getCloudShadowTexture())
		{
			if (!m_resourceManager->bind(directionalLightData->getCloudShadowTexture(), cloudShadowTexture))
				return 0;
		}

		return new DirectionalLightEntity(
			directionalLightData->getTransform(),
			directionalLightData->getSunColor(),
			directionalLightData->getBaseColor(),
			directionalLightData->getShadowColor(),
			cloudShadowTexture,
			directionalLightData->getCastShadow()
		);
	}

	if (const PointLightEntityData* pointLightData = dynamic_type_cast< const PointLightEntityData* >(&entityData))
	{
		return new PointLightEntity(
			pointLightData->getTransform(),
			pointLightData->getSunColor(),
			pointLightData->getBaseColor(),
			pointLightData->getShadowColor(),
			pointLightData->getRange(),
			pointLightData->getRandomFlickerAmount(),
			pointLightData->getRandomFlickerFilter()
		);
	}

	if (const SpotLightEntityData* spotLightData = dynamic_type_cast< const SpotLightEntityData* >(&entityData))
	{
		return new SpotLightEntity(
			spotLightData->getTransform(),
			spotLightData->getSunColor(),
			spotLightData->getBaseColor(),
			spotLightData->getShadowColor(),
			spotLightData->getRange(),
			spotLightData->getRadius(),
			spotLightData->getCastShadow()
		);
	}

	if (const SwitchEntityData* switchEntityData = dynamic_type_cast< const SwitchEntityData* >(&entityData))
	{
		Ref< SwitchEntity > switchEntity = new SwitchEntity(switchEntityData->getTransform(), 0);

		const RefArray< EntityData >& switchChildEntityData = switchEntityData->getEntityData();
		for (RefArray< EntityData >::const_iterator i = switchChildEntityData.begin(); i != switchChildEntityData.end(); ++i)
		{
			Ref< Entity > childEntity = builder->create(*i);
			if (childEntity)
				switchEntity->addEntity(childEntity);
		}

		return switchEntity;
	}

	if (const NullEntityData* nullData = dynamic_type_cast< const NullEntityData* >(&entityData))
		return new NullEntity(nullData->getTransform());

	if (const VolumeEntityData* volumeData = dynamic_type_cast< const VolumeEntityData* >(&entityData))
		return new VolumeEntity(volumeData);

	if (const ComponentEntityData* componentData = dynamic_type_cast< const ComponentEntityData* >(&entityData))
	{
		Ref< ComponentEntity > componentEntity = new ComponentEntity();
		for (uint32_t i = 0; i < componentData->m_components.size(); ++i)
		{
			Ref< IEntityComponent > component = builder->create(componentEntity, componentData->m_components[i]);
			if (!component)
				return 0;

			componentEntity->setComponent(component);
		}
		return componentEntity;
	}

	return 0;
}

Ref< IEntityEvent > WorldEntityFactory::createEntityEvent(const IEntityBuilder* builder, const IEntityEventData& entityEventData) const
{
	if (const DecalEventData* decalData = dynamic_type_cast< const DecalEventData* >(&entityEventData))
	{
		Ref< DecalEvent > decal = new DecalEvent();

		decal->m_size = decalData->getSize();
		decal->m_thickness = decalData->getThickness();
		decal->m_alpha = decalData->getAlpha();
		decal->m_cullDistance = decalData->getCullDistance();

		if (m_resourceManager->bind(decalData->getShader(), decal->m_shader))
			return decal;
	}

	return 0;
}

Ref< IEntityComponent > WorldEntityFactory::createEntityComponent(const world::IEntityBuilder* builder, Entity* owner, const IEntityComponentData& entityComponentData) const
{
	if (const ScriptComponentData* scriptComponentData = dynamic_type_cast< const ScriptComponentData* >(&entityComponentData))
		return scriptComponentData->createComponent(owner, m_resourceManager);

	return 0;
}

	}
}
