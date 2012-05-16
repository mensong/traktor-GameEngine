#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Math/Format.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Instance.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Model/Model.h"
#include "Model/Utilities.h"
#include "Model/Formats/ModelFormat.h"
#include "Physics/Mesh.h"
#include "Physics/MeshResource.h"
#include "Physics/Editor/MeshAsset.h"
#include "Physics/Editor/MeshPipeline.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.MeshPipeline", 2, MeshPipeline, editor::IPipeline)

bool MeshPipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getProperty< PropertyString >(L"Pipeline.AssetPath", L"");
	return true;
}

void MeshPipeline::destroy()
{
}

TypeInfoSet MeshPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< MeshAsset >());
	return typeSet;
}

bool MeshPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	Ref< const Object >& outBuildParams
) const
{
	const MeshAsset* meshAsset = checked_type_cast< const MeshAsset* >(sourceAsset);
	Path fileName = FileSystem::getInstance().getAbsolutePath(m_assetPath, meshAsset->getFileName());
	pipelineDepends->addDependency(fileName);
	return true;
}

bool MeshPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset,
	uint32_t sourceAssetHash,
	const Object* buildParams,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	uint32_t reason
) const
{
	const MeshAsset* meshAsset = checked_type_cast< const MeshAsset* >(sourceAsset);
	Ref< model::Model > model;

	// We allow models to be passed as build parameters in case models
	// are procedurally generated.
	if (buildParams)
	{
		log::info << L"Using parameter model" << Endl;
		model = checked_type_cast< model::Model* >(
			const_cast< Object* >(buildParams)
		);
	}
	else
	{
		// Import source model.
		Path fileName = FileSystem::getInstance().getAbsolutePath(m_assetPath, meshAsset->getFileName());
		model = model::ModelFormat::readAny(fileName);
	}

	if (!model)
	{
		log::error << L"Phys mesh pipeline failed; no model" << Endl;
		return false;
	}
	if (model->getPositions().empty() || model->getVertices().empty() || model->getPolygons().empty())
	{
		log::error << L"Phys mesh pipeline failed; no geometry" << Endl;
		return false;
	}

	model->clear(model::Model::CfMaterials | model::Model::CfColors | model::Model::CfNormals | model::Model::CfTexCoords | model::Model::CfJoints);
	model::cleanDuplicates(*model);
	model::triangulateModel(*model);

	// Calculate bounding box; used for center of gravity estimation.
	Aabb3 boundingBox = model::calculateModelBoundingBox(*model);
	Vector4 centerOfGravity = boundingBox.getCenter().xyz0();

	// Create physics mesh.
	AlignedVector< Vector4 > positions = model->getPositions();
	for (AlignedVector< Vector4 >::iterator i = positions.begin(); i != positions.end(); ++i)
		*i -= centerOfGravity;

	std::vector< Mesh::Triangle > meshShapeTriangles;
	std::vector< Mesh::Triangle > meshHullTriangles;

	const std::vector< model::Polygon >& shapeTriangles = model->getPolygons();
	for (std::vector< model::Polygon >::const_iterator i = shapeTriangles.begin(); i != shapeTriangles.end(); ++i)
	{
		T_ASSERT (i->getVertices().size() == 3);

		Mesh::Triangle shapeTriangle;
		for (int j = 0; j < 3; ++j)
			shapeTriangle.indices[j] = model->getVertex(i->getVertex(j)).getPosition();

		meshShapeTriangles.push_back(shapeTriangle);
	}

	if (meshAsset->m_calculateConvexHull)
	{
		log::info << L"Calculating convex hull..." << Endl;

		model::Model hull = *model;
		model::calculateConvexHull(hull);

		const std::vector< model::Polygon >& hullTriangles = hull.getPolygons();
		for (std::vector< model::Polygon >::const_iterator i = hullTriangles.begin(); i != hullTriangles.end(); ++i)
		{
			T_ASSERT (i->getVertices().size() == 3);

			Mesh::Triangle hullTriangle;
			for (int j = 0; j < 3; ++j)
				hullTriangle.indices[j] = hull.getVertex(i->getVertex(j)).getPosition();

			meshHullTriangles.push_back(hullTriangle);
		}

		// Improve center of gravity by weighting in volumes of each hull face tetrahedron.
		Vector4 Voffset = Vector4::zero();
		float Vtotal = 0.0f;
		for (std::vector< Mesh::Triangle >::const_iterator i = meshHullTriangles.begin(); i != meshHullTriangles.end(); ++i)
		{
			const Vector4 a = positions[i->indices[0]];
			const Vector4 b = positions[i->indices[1]];
			const Vector4 c = positions[i->indices[2]];

			float V = abs(dot3(a, cross(b, c))) / 6.0f;
			Vector4 C = (a + b + c) / Scalar(4.0f);

			Voffset += C * Scalar(V);
			Vtotal += V;
		}

		centerOfGravity += Voffset / Scalar(Vtotal);
	}

	// Log statistics.
	log::info << positions.size() << L" vertex(es)" << Endl;
	log::info << meshShapeTriangles.size() << L" shape triangle(s)" << Endl;
	log::info << meshHullTriangles.size() << L" hull triangle(s)" << Endl;
	log::info << L"Offset " << centerOfGravity << Endl;

	Mesh mesh;
	mesh.setVertices(positions);
	mesh.setShapeTriangles(meshShapeTriangles);
	mesh.setHullTriangles(meshHullTriangles);
	mesh.setOffset(centerOfGravity);

	Ref< db::Instance > instance = pipelineBuilder->createOutputInstance(
		outputPath,
		outputGuid		
	);
	if (!instance)
		return false;

	instance->setObject(new MeshResource());

	Ref< IStream > stream = instance->writeData(L"Data");
	mesh.write(stream);
	stream->close();

	return instance->commit();
}

Ref< ISerializable > MeshPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset
) const
{
	T_FATAL_ERROR;
	return 0;
}

	}
}
