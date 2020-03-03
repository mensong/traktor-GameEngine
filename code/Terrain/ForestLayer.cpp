#include <limits>
#include "Core/Containers/StaticVector.h"
#include "Core/Log/Log.h"
#include "Core/Math/Half.h"
#include "Core/Math/RandomGeometry.h"
#include "Heightfield/Heightfield.h"
#include "Render/ISimpleTexture.h"
#include "Render/Context/RenderContext.h"
#include "Resource/IResourceManager.h"
#include "Terrain/ForestLayer.h"
#include "Terrain/ForestLayerData.h"
#include "Terrain/Terrain.h"
#include "Terrain/TerrainComponent.h"
#include "Terrain/TerrainSurfaceCache.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldBuildContext.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace terrain
	{
		namespace
		{

const render::Handle s_handleNormals(L"Normals");
const render::Handle s_handleHeightfield(L"Heightfield");
const render::Handle s_handleSurface(L"Surface");
const render::Handle s_handleWorldExtent(L"WorldExtent");
const render::Handle s_handleEye(L"Eye");

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.ForestLayer", ForestLayer, ITerrainLayer)

ForestLayer::ForestLayer()
{
}

bool ForestLayer::create(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	const ForestLayerData& layerData,
	const TerrainComponent& terrainComponent
)
{
	if (!resourceManager->bind(layerData.m_lod0mesh, m_lod0mesh))
		return false;
	if (!resourceManager->bind(layerData.m_lod1mesh, m_lod1mesh))
		return false;

	m_data = layerData;
	updatePatches(terrainComponent);
	return true;
}

void ForestLayer::update(const world::UpdateParams& update)
{
}

void ForestLayer::build(
	TerrainComponent& terrainComponent,
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass
)
{
	const resource::Proxy< Terrain >& terrain = terrainComponent.getTerrain();

	// Update clusters at first pass from eye pow.
	bool updateClusters = bool((worldRenderPass.getPassFlags() & world::IWorldRenderPass::PfFirst) != 0);

	Matrix44 view = worldRenderView.getView();
	Vector4 eye = view.inverse().translation();

	if (updateClusters)
	{
		// Brute force test.

		m_lod0indices.resize(0);
		m_lod1indices.resize(0);

		for (uint32_t i = 0; i < (uint32_t)m_trees.size(); ++i)
		{
			const auto& tree = m_trees[i];
			Scalar distance = (tree.position - eye).length();
			if (distance < m_data.m_lod0distance)
				m_lod0indices.push_back(i);
			else if (distance < m_data.m_lod1distance)
				m_lod1indices.push_back(i);
		}
	}

	render::RenderContext* renderContext = context.getRenderContext();

	// Expose some more shader parameters, such as terrain color etc.
	render::ProgramParameters* extraParameters = renderContext->alloc< render::ProgramParameters >();
	extraParameters->beginParameters(renderContext);
	extraParameters->setTextureParameter(s_handleNormals, terrain->getNormalMap());
	extraParameters->setTextureParameter(s_handleHeightfield, terrain->getHeightMap());
	extraParameters->setTextureParameter(s_handleSurface, terrainComponent.getSurfaceCache()->getBaseTexture());
	extraParameters->setVectorParameter(s_handleWorldExtent, terrain->getHeightfield()->getWorldExtent());
	extraParameters->setVectorParameter(s_handleEye, eye);
	extraParameters->endParameters(renderContext);

	for (uint32_t i = 0; i < m_lod1indices.size(); )
	{
		uint32_t batch = std::min< uint32_t >(m_lod1indices.size() - i, mesh::InstanceMesh::MaxInstanceCount);

		m_instanceData.resize(batch);
		for (int32_t j = 0; j < batch; ++j, ++i)
		{
			m_trees[m_lod1indices[i]].rotation.e.storeAligned(m_instanceData[j].data.rotation);
			m_trees[m_lod1indices[i]].position.storeAligned(m_instanceData[j].data.translation);
			m_instanceData[j].data.scale = m_trees[m_lod1indices[i]].scale;
			m_instanceData[j].distance = 0.0f;
		}

		m_lod1mesh->build(
			renderContext,
			worldRenderPass,
			m_instanceData,
			extraParameters
		);
	}

	for (uint32_t i = 0; i < m_lod0indices.size(); )
	{
		uint32_t batch = std::min< uint32_t >(m_lod0indices.size() - i, mesh::InstanceMesh::MaxInstanceCount);

		m_instanceData.resize(batch);
		for (int32_t j = 0; j < batch; ++j, ++i)
		{
			m_trees[m_lod0indices[i]].rotation.e.storeAligned(m_instanceData[j].data.rotation);
			m_trees[m_lod0indices[i]].position.storeAligned(m_instanceData[j].data.translation);
			m_instanceData[j].data.scale = m_trees[m_lod0indices[i]].scale;
			m_instanceData[j].distance = 0.0f;
		}

		m_lod0mesh->build(
			renderContext,
			worldRenderPass,
			m_instanceData,
			extraParameters
		);
	}
}

void ForestLayer::updatePatches(const TerrainComponent& terrainComponent)
{
	const auto& terrain = terrainComponent.getTerrain();
	const auto& heightfield = terrain->getHeightfield();
	const float densityInv = 1.0f / m_data.m_density;
	const int32_t size = heightfield->getSize();
	const Vector4 extentPerGrid = heightfield->getWorldExtent() / Scalar(float(size));
	Random random;

	m_trees.resize(0);
	for (float z = 0; z < size; z += densityInv)
	{
		for (float x = 0; x < size; x += densityInv)
		{
			// Check if trees are allowed on this position.
			if (heightfield->getGridMaterial(x, z) != m_data.m_material)
				continue;

			// Get world position.
			float wx, wy, wz;
			heightfield->gridToWorld(x, z, wx, wz);
			wy = heightfield->getWorldHeight(wx, wz);
			wx += extentPerGrid.x() * densityInv * (random.nextFloat() - 0.5f);
			wz += extentPerGrid.z() * densityInv * (random.nextFloat() - 0.5f);

			// Calculate rotation.
			float gx, gz;
			heightfield->worldToGrid(wx, wz, gx, gz);
			Vector4 normal = heightfield->normalAt(gx, gz);
			float rx = (random.nextFloat() * 2.0f - 1.0f) * m_data.m_randomTilt;
			float rz = (random.nextFloat() * 2.0f - 1.0f) * m_data.m_randomTilt;
			float head = random.nextFloat() * TWO_PI;
			Quaternion Qu = slerp(Quaternion::identity(), Quaternion(Vector4(0.0f, 1.0f, 0.0f), normal), m_data.m_upness);
			Quaternion Qr = Quaternion::fromAxisAngle(Vector4(1.0f, 0.0f, 0.0f), rx) * Quaternion::fromAxisAngle(Vector4(0.0f, 0.0f, 1.0f), rz);
			Quaternion Qh = Quaternion::fromAxisAngle(Vector4(0.0f, 1.0f, 0.0f), head);

			// Add tree on position.
			auto& tree = m_trees.push_back();
			tree.position = Vector4(wx, wy, wz, 1.0f);
			tree.rotation = Qr * Qu * Qh;
			tree.scale = random.nextFloat() * m_data.m_randomScale + (1.0f - m_data.m_randomScale);
		}
	}
}

	}
}
