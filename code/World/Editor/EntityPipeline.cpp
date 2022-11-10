/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Reflection/Reflection.h"
#include "Core/Reflection/RfpMemberType.h"
#include "Core/Reflection/RfmObject.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Database/Instance.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "World/EntityData.h"
#include "World/IEntityComponentData.h"
#include "World/IEntityEventData.h"
#include "World/Editor/EditorAttributesComponentData.h"
#include "World/Editor/EntityPipeline.h"

namespace traktor
{
	namespace world
	{
		namespace
		{

bool removeMember(RfmCompound* owner, ReflectionMember* member)
{
	if (owner->removeMember(member))
		return true;

	// Unable to remove from this compound, recurse with each member compound.
	for (auto c : owner->getMembers())
	{
		if (auto cc = dynamic_type_cast< RfmCompound* >(c))
		{
			if (removeMember(cc, member))
				return true;
		}
	}

	return false;
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.EntityPipeline", 1, EntityPipeline, editor::IPipeline)

bool EntityPipeline::create(const editor::IPipelineSettings* settings)
{
	m_editor = settings->getPropertyIncludeHash< bool >(L"Pipeline.TargetEditor", false);
	return true;
}

void EntityPipeline::destroy()
{
}

TypeInfoSet EntityPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert< EntityData >();
	typeSet.insert< IEntityEventData >();
	typeSet.insert< IEntityComponentData >();
	return typeSet;
}

bool EntityPipeline::shouldCache() const
{
	return false;
}

uint32_t EntityPipeline::hashAsset(const ISerializable* sourceAsset) const
{
	return DeepHash(sourceAsset).get();
}

bool EntityPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	Ref< Reflection > reflection = Reflection::create(sourceAsset);
	if (!reflection)
		return false;

	// Find all members which reference child entities.
	RefArray< ReflectionMember > objectMembers;
	reflection->findMembers(RfpMemberType(type_of< RfmObject >()), objectMembers);

	while (!objectMembers.empty())
	{
		Ref< const RfmObject > objectMember = checked_type_cast< const RfmObject*, false >(objectMembers.front());
		objectMembers.pop_front();

		if (const EntityData* entityData = dynamic_type_cast< const EntityData* >(objectMember->get()))
		{
			pipelineDepends->addDependency(entityData);
		}
		else if (const IEntityEventData* entityEventData = dynamic_type_cast< const IEntityEventData* >(objectMember->get()))
		{
			pipelineDepends->addDependency(entityEventData);
		}
		else if (const IEntityComponentData* entityComponentData = dynamic_type_cast< const IEntityComponentData* >(objectMember->get()))
		{
			pipelineDepends->addDependency(entityComponentData);
		}
		else if (objectMember->get())
		{
			Ref< Reflection > childReflection = Reflection::create(objectMember->get());
			if (childReflection)
				childReflection->findMembers(RfpMemberType(type_of< RfmObject >()), objectMembers);
		}
	}

	return true;
}

bool EntityPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const editor::PipelineDependencySet* dependencySet,
	const editor::PipelineDependency* dependency,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	const Object* buildParams,
	uint32_t reason
) const
{
	Ref< ISerializable > outputAsset = pipelineBuilder->buildProduct(sourceInstance, sourceAsset, buildParams);
	if (!outputAsset)
		return false;

	Ref< db::Instance > outputInstance = pipelineBuilder->createOutputInstance(outputPath, outputGuid);
	if (!outputInstance)
		return false;

	outputInstance->setObject(outputAsset);

	if (!outputInstance->commit())
		return false;

	return true;
}

Ref< ISerializable > EntityPipeline::buildProduct(
	editor::IPipelineBuilder* pipelineBuilder,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const Object* buildParams
) const
{
	// Ensure we have a reflection of source asset.
	Ref< Reflection > reflection = Reflection::create(sourceAsset);
	if (!reflection)
		return nullptr;

	RefArray< ReflectionMember > objectMembers;
	reflection->findMembers(RfpMemberType(type_of< RfmObject >()), objectMembers);

	// Remove editor attributes component; can not be used in runtime.
	if (!m_editor)
	{
		for (;;)
		{
			auto it = std::find_if(objectMembers.begin(), objectMembers.end(), [](ReflectionMember* member) {
				return is_a< EditorAttributesComponentData >(static_cast< RfmObject* >(member)->get());
			});
			if (it != objectMembers.end())
			{
				ReflectionMember* member = *it;

				auto attributes = static_cast< const EditorAttributesComponentData* >(static_cast< RfmObject* >(member)->get());
				if (!attributes->include)
					return nullptr;

				removeMember(reflection, member);
				objectMembers.erase(it);
			}
			else
				break;
		}
	}

	auto ownerEntityData = dynamic_type_cast< const EntityData* >(sourceAsset);
	while (!objectMembers.empty())
	{
		Ref< RfmObject > objectMember = checked_type_cast< RfmObject*, false >(objectMembers.front());
		objectMembers.pop_front();

		if (auto entityData = dynamic_type_cast< const EntityData* >(objectMember->get()))
		{
			// Build entity trough pipeline; replace entity with product.
			objectMember->set(pipelineBuilder->buildProduct(sourceInstance, entityData));
		}
		else if (auto entityComponentData = dynamic_type_cast< const IEntityComponentData* >(objectMember->get()))
		{
			// Build component trough pipeline; replace component with product.
			objectMember->set(pipelineBuilder->buildProduct(sourceInstance, entityComponentData, ownerEntityData));
		}
		else if (auto entityEventData = dynamic_type_cast< const IEntityEventData* >(objectMember->get()))
		{
			// Build event trough pipeline; replace event with product.
			objectMember->set(pipelineBuilder->buildProduct(sourceInstance, entityEventData, ownerEntityData));
		}
		else if (objectMember->get())
		{
			// Scan recursively through object references; add to member list.
			Ref< Reflection > childReflection = Reflection::create(objectMember->get());
			if (childReflection)
				childReflection->findMembers(RfpMemberType(type_of< RfmObject >()), objectMembers);
		}
	}

	return reflection->clone();
}

	}
}
