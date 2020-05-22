#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/Boxes/BoxedAabb3.h"
#include "Core/Class/Boxes/BoxedColor4f.h"
#include "Core/Class/Boxes/BoxedRefArray.h"
#include "Core/Class/Boxes/BoxedTypeInfo.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "World/EntityBuilder.h"
#include "World/EntityBuilderWithSchema.h"
#include "World/EntitySchema.h"
#include "World/IEntityEvent.h"
#include "World/IEntityEventData.h"
#include "World/IEntityEventInstance.h"
#include "World/EntityEventManager.h"
#include "World/IEntityFactory.h"
#include "World/IEntityRenderer.h"
#include "World/IWorldRenderer.h"
#include "World/WorldClassFactory.h"
#include "World/Entity/CameraComponent.h"
#include "World/Entity/CameraComponentData.h"
#include "World/Entity.h"
#include "World/EntityData.h"
#include "World/Entity/EventSetComponent.h"
#include "World/Entity/EventSetComponentData.h"
#include "World/Entity/FacadeComponent.h"
#include "World/Entity/FacadeComponentData.h"
#include "World/Entity/GroupComponent.h"
#include "World/Entity/GroupEntity.h"
#include "World/Entity/LightComponent.h"
#include "World/Entity/LightComponentData.h"
#include "World/Entity/ScriptComponent.h"
#include "World/Entity/ScriptComponentData.h"
#include "World/Entity/VolumeComponent.h"
#include "World/Entity/VolumeComponentData.h"

namespace traktor
{
	namespace world
	{
		namespace
		{

void IEntityEventInstance_cancelImmediate(IEntityEventInstance* self)
{
	self->cancel(Cancel::Immediate);
}

void IEntityEventInstance_cancelEnd(IEntityEventInstance* self)
{
	self->cancel(Cancel::End);
}

IEntityEventInstance* EntityEventManager_raise_1(EntityEventManager* self, const IEntityEvent* event, Entity* sender, const Transform& Toffset)
{
	return self->raise(event, sender, Toffset);
}

void EntityEventManager_cancelAllImmediate(EntityEventManager* self)
{
	self->cancelAll(Cancel::Immediate);
}

void EntityEventManager_cancelAllEnd(EntityEventManager* self)
{
	self->cancelAll(Cancel::End);
}

Ref< Entity > IEntityBuilder_create(IEntityBuilder* self, const EntityData* entityData)
{
	return self->create(entityData);
}

void Entity_setTransform(Entity* self, const Transform& transform)
{
	self->setTransform(transform);
}

Transform Entity_getTransform(Entity* self)
{
	return self->getTransform();
}

void CameraComponentData_setProjection(CameraComponentData* self, int32_t type)
{
	self->setProjection((Projection)type);
}

int32_t CameraComponentData_getProjection(CameraComponentData* self)
{
	return (int32_t)self->getProjection();
}

void CameraComponent_setProjection(CameraComponent* self, int32_t type)
{
	self->setProjection((Projection)type);
}

int32_t CameraComponent_getProjection(CameraComponent* self)
{
	return (int32_t)self->getProjection();
}

RefArray< Entity > GroupComponent_getEntitiesOf(GroupComponent* self, const TypeInfo& entityType)
{
	RefArray< Entity > entities;
	self->getEntitiesOf(entityType, entities);
	return entities;
}

Ref< Entity > GroupComponent_getFirstEntityOf(GroupComponent* self, const TypeInfo& entityType)
{
	return self->getFirstEntityOf(entityType);
}

RefArray< Entity > GroupEntity_getEntitiesOf(GroupEntity* self, const TypeInfo& entityType)
{
	RefArray< Entity > entities;
	self->getEntitiesOf(entityType, entities);
	return entities;
}

Ref< Entity > GroupEntity_getFirstEntityOf(GroupEntity* self, const TypeInfo& entityType)
{
	return self->getFirstEntityOf(entityType);
}

void Entity_update(Entity* self, float totalTime, float deltaTime)
{
	UpdateParams up;
	up.totalTime = totalTime;
	up.deltaTime = deltaTime;
	up.alternateTime = totalTime;
	self->update(up);
}

IEntityComponentData* EntityData_getComponent(EntityData* self, const TypeInfo& componentDataType)
{
	return self->getComponent(componentDataType);
}

IEntityComponent* Entity_getComponent(Entity* self, const TypeInfo& componentType)
{
	return self->getComponent(componentType);
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.WorldClassFactory", 0, WorldClassFactory, IRuntimeClassFactory)

void WorldClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	auto classIEntityEventInstance = new AutoRuntimeClass< IEntityEventInstance >();
	classIEntityEventInstance->addMethod("cancelImmediate", &IEntityEventInstance_cancelImmediate);
	classIEntityEventInstance->addMethod("cancelEnd", &IEntityEventInstance_cancelEnd);
	registrar->registerClass(classIEntityEventInstance);

	auto classIEntityEventData = new AutoRuntimeClass< IEntityEventData >();
	registrar->registerClass(classIEntityEventData);

	auto classIEntityEvent = new AutoRuntimeClass< IEntityEvent >();
	registrar->registerClass(classIEntityEvent);

	auto classEntityEventManager = new AutoRuntimeClass< EntityEventManager >();
	classEntityEventManager->addMethod("raise", &EntityEventManager_raise_1);
	classEntityEventManager->addMethod("cancelAllImmediate", &EntityEventManager_cancelAllImmediate);
	classEntityEventManager->addMethod("cancelAllEnd", &EntityEventManager_cancelAllEnd);
	registrar->registerClass(classEntityEventManager);

	auto classEntitySchema = new AutoRuntimeClass< EntitySchema >();
	classEntitySchema->addMethod< Entity*, uint32_t >("getEntity", &EntitySchema::getEntity);
	classEntitySchema->addMethod< Entity*, const std::wstring&, uint32_t >("getEntity", &EntitySchema::getEntity);
	classEntitySchema->addMethod< Entity*, const TypeInfo&, uint32_t >("getEntityOf", &EntitySchema::getEntity);
	classEntitySchema->addMethod< Entity*, const std::wstring&, const TypeInfo&, uint32_t >("getEntityOf", &EntitySchema::getEntity);
	classEntitySchema->addMethod< Entity*, const Entity*, uint32_t >("getChildEntity", &EntitySchema::getChildEntity);
	classEntitySchema->addMethod< Entity*, const Entity*, const std::wstring&, uint32_t >("getChildEntity", &EntitySchema::getChildEntity);
	classEntitySchema->addMethod< Entity*, const Entity*, const TypeInfo&, uint32_t >("getChildEntityOf", &EntitySchema::getChildEntity);
	classEntitySchema->addMethod< Entity*, const Entity*, const std::wstring&, const TypeInfo&, uint32_t >("getChildEntityOf", &EntitySchema::getChildEntity);
	registrar->registerClass(classEntitySchema);

	auto classIEntityFactory = new AutoRuntimeClass< IEntityFactory >();
	registrar->registerClass(classIEntityFactory);

	auto classIEntityBuilder = new AutoRuntimeClass< IEntityBuilder >();
	classIEntityBuilder->addMethod("addFactory", &IEntityBuilder::addFactory);
	classIEntityBuilder->addMethod("removeFactory", &IEntityBuilder::removeFactory);
	classIEntityBuilder->addMethod("create", &IEntityBuilder_create);
	registrar->registerClass(classIEntityBuilder);

	auto classEntityBuilder = new AutoRuntimeClass< EntityBuilder >();
	classEntityBuilder->addConstructor();
	registrar->registerClass(classEntityBuilder);

	auto classEntityBuilderWithSchema = new AutoRuntimeClass< EntityBuilderWithSchema >();
	classEntityBuilderWithSchema->addConstructor< IEntityBuilder*, EntitySchema* >();
	registrar->registerClass(classEntityBuilderWithSchema);

	auto classIEntityRenderer = new AutoRuntimeClass< IEntityRenderer >();
	registrar->registerClass(classIEntityRenderer);

	auto classEntityData = new AutoRuntimeClass< EntityData >();
	classEntityData->addConstructor();
	classEntityData->addProperty("name", &EntityData::setName, &EntityData::getName);
	classEntityData->addProperty("transform", &EntityData::setTransform, &EntityData::getTransform);
	classEntityData->addMethod("setComponent", &EntityData::setComponent);
	classEntityData->addMethod("getComponent", &EntityData_getComponent);
	registrar->registerClass(classEntityData);

	auto classEntity = new AutoRuntimeClass< Entity >();
	classEntity->addConstructor();
	classEntity->addConstructor< const Transform& >();
	classEntity->addProperty("transform", &Entity_setTransform, &Entity_getTransform);
	classEntity->addProperty("boundingBox", &Entity::getBoundingBox);
	classEntity->addMethod("destroy", &Entity::destroy);
	classEntity->addMethod("update", &Entity_update);
	classEntity->addMethod("setComponent", &Entity::setComponent);
	classEntity->addMethod("getComponent", &Entity_getComponent);
	registrar->registerClass(classEntity);

	auto classGroupComponent = new AutoRuntimeClass< GroupComponent >();
	classGroupComponent->addConstructor();
	classGroupComponent->addMethod("addEntity", &GroupComponent::addEntity);
	classGroupComponent->addMethod("removeEntity", &GroupComponent::removeEntity);
	classGroupComponent->addMethod("removeAllEntities", &GroupComponent::removeAllEntities);
	classGroupComponent->addMethod("getEntities", &GroupComponent::getEntities);
	classGroupComponent->addMethod("getEntitiesOf", &GroupComponent_getEntitiesOf);
	classGroupComponent->addMethod("getFirstEntityOf", &GroupComponent_getFirstEntityOf);
	registrar->registerClass(classGroupComponent);

	auto classGroupEntity = new AutoRuntimeClass< GroupEntity >();
	classGroupEntity->addConstructor();
	classGroupEntity->addConstructor< const Transform&/*, uint32_t*/ >();
	classGroupEntity->addMethod("addEntity", &GroupEntity::addEntity);
	classGroupEntity->addMethod("removeEntity", &GroupEntity::removeEntity);
	classGroupEntity->addMethod("removeAllEntities", &GroupEntity::removeAllEntities);
	classGroupEntity->addMethod("getEntities", &GroupEntity::getEntities);
	classGroupEntity->addMethod("getEntitiesOf", &GroupEntity_getEntitiesOf);
	classGroupEntity->addMethod("getFirstEntityOf", &GroupEntity_getFirstEntityOf);
	registrar->registerClass(classGroupEntity);

	auto classIEntityComponentData = new AutoRuntimeClass< IEntityComponentData >();
	registrar->registerClass(classIEntityComponentData);

	auto classIEntityComponent = new AutoRuntimeClass< IEntityComponent >();
	classIEntityComponent->addProperty("boundingBox", &IEntityComponent::getBoundingBox);
	classIEntityComponent->addMethod("setTransform", &IEntityComponent::setTransform);
	registrar->registerClass(classIEntityComponent);

	auto classCameraComponentData = new AutoRuntimeClass< CameraComponentData >();
	classCameraComponentData->addConstant("Orthographic", Any::fromInt32((int32_t)Projection::Orthographic));
	classCameraComponentData->addConstant("Perspective", Any::fromInt32((int32_t)Projection::Perspective));
	classCameraComponentData->addConstructor();
	classCameraComponentData->addProperty("projection", &CameraComponentData_setProjection, &CameraComponentData_getProjection);
	classCameraComponentData->addProperty("fieldOfView", &CameraComponentData::setFieldOfView, &CameraComponentData::getFieldOfView);
	classCameraComponentData->addProperty("width", &CameraComponentData::setWidth, &CameraComponentData::getWidth);
	classCameraComponentData->addProperty("height", &CameraComponentData::setHeight, &CameraComponentData::getHeight);
	registrar->registerClass(classCameraComponentData);

	auto classCameraComponent = new AutoRuntimeClass< CameraComponent >();
	classCameraComponent->addConstant("Orthographic", Any::fromInt32((int32_t)Projection::Orthographic));
	classCameraComponent->addConstant("Perspective", Any::fromInt32((int32_t)Projection::Perspective));
	classCameraComponent->addProperty("projection", &CameraComponent_setProjection, &CameraComponent_getProjection);
	classCameraComponent->addProperty("fieldOfView", &CameraComponent::setFieldOfView, &CameraComponent::getFieldOfView);
	classCameraComponent->addProperty("width", &CameraComponent::setWidth, &CameraComponent::getWidth);
	classCameraComponent->addProperty("height", &CameraComponent::setHeight, &CameraComponent::getHeight);
	registrar->registerClass(classCameraComponent);

	auto classLightComponentData = new AutoRuntimeClass< LightComponentData >();
	classLightComponentData->addProperty("color", &LightComponentData::setColor, &LightComponentData::getColor);
	classLightComponentData->addProperty("castShadow", &LightComponentData::setCastShadow, &LightComponentData::getCastShadow);
	classLightComponentData->addProperty("range", &LightComponentData::setRange, &LightComponentData::getRange);
	classLightComponentData->addProperty("radius", &LightComponentData::setRadius, &LightComponentData::getRadius);
	classLightComponentData->addProperty("flickerAmount", &LightComponentData::setFlickerAmount, &LightComponentData::getFlickerAmount);
	classLightComponentData->addProperty("flickerFilter", &LightComponentData::setFlickerFilter, &LightComponentData::getFlickerFilter);
	registrar->registerClass(classLightComponentData);

	auto classLightComponent = new AutoRuntimeClass< LightComponent >();
	classLightComponent->addProperty("color", &LightComponent::setColor, &LightComponent::getColor);
	classLightComponent->addProperty("castShadow", &LightComponent::setCastShadow, &LightComponent::getCastShadow);
	classLightComponent->addProperty("range", &LightComponent::setRange, &LightComponent::getRange);
	classLightComponent->addProperty("radius", &LightComponent::setRadius, &LightComponent::getRadius);
	classLightComponent->addProperty("flickerAmount", &LightComponent::setFlickerAmount, &LightComponent::getFlickerAmount);
	classLightComponent->addProperty("flickerFilter", &LightComponent::setFlickerFilter, &LightComponent::getFlickerFilter);
	registrar->registerClass(classLightComponent);

	auto classScriptComponentData = new AutoRuntimeClass< ScriptComponentData >();
	registrar->registerClass(classScriptComponentData);

	auto classScriptComponent = new AutoRuntimeClass< ScriptComponent >();
	registrar->registerClass(classScriptComponent);

	auto classVolumeComponentData = new AutoRuntimeClass< VolumeComponentData >();
	registrar->registerClass(classVolumeComponentData);

	auto classVolumeComponent = new AutoRuntimeClass< VolumeComponent >();
	classVolumeComponent->addMethod("inside", &VolumeComponent::inside);
	registrar->registerClass(classVolumeComponent);

	auto classEventSetComponentData = new AutoRuntimeClass< EventSetComponentData >();
	registrar->registerClass(classEventSetComponentData);

	auto classEventSetComponent = new AutoRuntimeClass< EventSetComponent >();
	classEventSetComponent->addMethod("getEvent", &EventSetComponent::getEvent);
	registrar->registerClass(classEventSetComponent);

	auto classFacadeComponentData = new AutoRuntimeClass< FacadeComponentData >();
	registrar->registerClass(classFacadeComponentData);

	auto classFacadeComponent = new AutoRuntimeClass< FacadeComponent >();
	classFacadeComponent->addMethod("addEntity", &FacadeComponent::addEntity);
	classFacadeComponent->addMethod("removeEntity", &FacadeComponent::removeEntity);
	classFacadeComponent->addMethod("show", &FacadeComponent::show);
	classFacadeComponent->addMethod("showOnly", &FacadeComponent::showOnly);
	classFacadeComponent->addMethod("hide", &FacadeComponent::hide);
	classFacadeComponent->addMethod("hideAll", &FacadeComponent::hideAll);
	classFacadeComponent->addMethod("isVisible", &FacadeComponent::isVisible);
	registrar->registerClass(classFacadeComponent);

	auto classIWorldRenderer = new AutoRuntimeClass< IWorldRenderer >();
	classIWorldRenderer->addConstant("Disabled", Any::fromInt32((int32_t)Quality::Disabled));
	classIWorldRenderer->addConstant("Low", Any::fromInt32((int32_t)Quality::Low));
	classIWorldRenderer->addConstant("Medium", Any::fromInt32((int32_t)Quality::Medium));
	classIWorldRenderer->addConstant("High", Any::fromInt32((int32_t)Quality::High));
	classIWorldRenderer->addConstant("Ultra", Any::fromInt32((int32_t)Quality::Ultra));
	registrar->registerClass(classIWorldRenderer);
}

	}
}
