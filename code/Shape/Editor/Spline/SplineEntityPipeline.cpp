/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyString.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Mesh/MeshComponentData.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Model/Model.h"
#include "Model/ModelFormat.h"
#include "Physics/MeshShapeDesc.h"
#include "Physics/StaticBodyDesc.h"
#include "Physics/Editor/MeshAsset.h"
#include "Physics/World/RigidBodyComponentData.h"
#include "Shape/Editor/Spline/ControlPointComponentData.h"
#include "Shape/Editor/Spline/ExtrudeShapeLayerData.h"
#include "Render/Shader.h"
#include "Shape/Editor/Spline/SplineComponentData.h"
#include "Shape/Editor/Spline/SplineEntityPipeline.h"
#include "Shape/Editor/Spline/SplineEntityReplicator.h"
#include "World/EntityData.h"

namespace traktor
{
	namespace shape
	{
		namespace
		{

const resource::Id< render::Shader > c_defaultShader(Guid(L"{F01DE7F1-64CE-4613-9A17-899B44D5414E}"));

const Guid c_renderMeshIdSeed(L"{E5CE60FA-9C1A-4DB3-9A55-634EB157D1B9}");
//const Guid c_collisionShapeIdSeed(L"{4BF9C3FE-673B-4DA8-8DB0-6B4E0D8CFBA0}");

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.shape.SplineEntityPipeline", 3, SplineEntityPipeline, world::EntityPipeline)

bool SplineEntityPipeline::create(const editor::IPipelineSettings* settings)
{
	if (!world::EntityPipeline::create(settings))
		return false;

	m_assetPath = settings->getPropertyExcludeHash< std::wstring >(L"Pipeline.AssetPath");
	m_targetEditor = settings->getPropertyIncludeHash< bool >(L"Pipeline.TargetEditor");
	
	m_replicator = new SplineEntityReplicator();
	m_replicator->create(settings);

	return true;
}

TypeInfoSet SplineEntityPipeline::getAssetTypes() const
{
	return makeTypeInfoSet<
		ControlPointComponentData,
		ExtrudeShapeLayerData,
		SplineComponentData
	>();
}

bool SplineEntityPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	if (m_targetEditor)
		pipelineDepends->addDependency(c_defaultShader, editor::PdfBuild | editor::PdfResource);

	if (auto splineComponentData = dynamic_type_cast< const SplineComponentData* >(sourceAsset))
	{
		for (auto id : splineComponentData->getCollisionGroup())
			pipelineDepends->addDependency(id, editor::PdfBuild | editor::PdfResource);
		for (auto id : splineComponentData->getCollisionMask())
			pipelineDepends->addDependency(id, editor::PdfBuild | editor::PdfResource);	
	}

	if (auto extrudeShapeLayerData = dynamic_type_cast< const ExtrudeShapeLayerData* >(sourceAsset))
	{
		pipelineDepends->addDependency(extrudeShapeLayerData->getMeshStart(), editor::PdfBuild);
		pipelineDepends->addDependency(extrudeShapeLayerData->getMeshRepeat(), editor::PdfBuild);
		pipelineDepends->addDependency(extrudeShapeLayerData->getMeshEnd(), editor::PdfBuild);
	}

	return world::EntityPipeline::buildDependencies(pipelineDepends, sourceInstance, sourceAsset, outputPath, outputGuid);
}

Ref< ISerializable > SplineEntityPipeline::buildProduct(
	editor::IPipelineBuilder* pipelineBuilder,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const Object* buildParams
) const
{
	if (m_targetEditor)
	{
		// In editor we generate spline geometry dynamically thus
		// not necessary to explicitly build mesh when building for editor.
		return world::EntityPipeline::buildProduct(pipelineBuilder, sourceInstance, sourceAsset, buildParams);
	}

	if (auto splineComponentData = dynamic_type_cast<const SplineComponentData*>(sourceAsset))
	{
		const world::EntityData* owner = mandatory_non_null_type_cast< const world::EntityData* >(buildParams);

		Ref< model::Model > outputModel = m_replicator->createModel(pipelineBuilder, owner, splineComponentData, world::IEntityReplicator::Usage::Visual);
		if (!outputModel)
		{
			log::warning << L"Unable to create model from spline \"" << owner->getName() << L"\"." << Endl;
			return nullptr;
		}

		const Guid& entityId = owner->getId();
		if (entityId.isNull())
			return nullptr;

		const Guid outputRenderMeshGuid = entityId.permutation(c_renderMeshIdSeed);
		const std::wstring outputRenderMeshPath = L"Generated/" + outputRenderMeshGuid.format();

		// Build output mesh from merged model.
		Ref< mesh::MeshAsset > visualMeshAsset = new mesh::MeshAsset();
		pipelineBuilder->buildAdHocOutput(
		 	visualMeshAsset,
		 	outputRenderMeshPath,
		 	outputRenderMeshGuid,
		 	outputModel
		);

		// Replace mesh component referencing our merged mesh.
		return new mesh::MeshComponentData(
		 	resource::Id< mesh::IMesh >(outputRenderMeshGuid)
		);
	}

	// All other components should be discarded.
	return nullptr;
}

	}
}
