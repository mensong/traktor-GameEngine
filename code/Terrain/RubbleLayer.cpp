#include <limits>
#include "Core/Containers/StaticVector.h"
#include "Core/Log/Log.h"
#include "Core/Math/Half.h"
#include "Core/Math/RandomGeometry.h"
#include "Heightfield/Heightfield.h"
#include "Mesh/Instance/InstanceMesh.h"
#include "Render/ISimpleTexture.h"
#include "Render/Context/RenderContext.h"
#include "Resource/IResourceManager.h"
#include "Terrain/Terrain.h"
#include "Terrain/TerrainEntity.h"
#include "Terrain/TerrainSurfaceCache.h"
#include "Terrain/RubbleLayer.h"
#include "Terrain/RubbleLayerData.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldContext.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace terrain
	{
		namespace
		{

render::handle_t s_handleNormals;
render::handle_t s_handleHeightfield;
render::handle_t s_handleSurface;
render::handle_t s_handleWorldExtent;
render::handle_t s_handleEye;
render::handle_t s_handleMaxDistance;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.RubbleLayer", RubbleLayer, ITerrainLayer)

RubbleLayer::RubbleLayer()
:	m_clusterSize(0.0f)
{
	s_handleNormals = render::getParameterHandle(L"Normals");
	s_handleHeightfield = render::getParameterHandle(L"Heightfield");
	s_handleSurface = render::getParameterHandle(L"Surface");
	s_handleWorldExtent = render::getParameterHandle(L"WorldExtent");
	s_handleEye = render::getParameterHandle(L"Eye");
	s_handleMaxDistance = render::getParameterHandle(L"MaxDistance");
}

bool RubbleLayer::create(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	const RubbleLayerData& layerData,
	const TerrainEntity& terrainEntity
)
{
	m_layerData = layerData;

	if (!resourceManager->bind(m_layerData.m_mesh, m_mesh))
		return false;

	updatePatches(terrainEntity);
	return true;
}

void RubbleLayer::update(const world::UpdateParams& update)
{
}

void RubbleLayer::render(
	TerrainEntity& terrainEntity,
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass
)
{
	const resource::Proxy< Terrain >& terrain = terrainEntity.getTerrain();

	// \fixme Assume depth pass enabled; need some information about first pass from camera POV.
	bool updateClusters = bool(
		worldRenderPass.getTechnique() == render::getParameterHandle(L"World_DepthWrite") ||
		worldRenderPass.getTechnique() == render::getParameterHandle(L"World_PreLitGBufferWrite") ||
		worldRenderPass.getTechnique() == render::getParameterHandle(L"World_DeferredGBufferWrite")
	);

	const Matrix44& view = worldRenderView.getView();
	Vector4 eye = view.inverse().translation();

	if (updateClusters)
	{
		Frustum viewFrustum = worldRenderView.getViewFrustum();
		viewFrustum.setFarZ(Scalar(m_layerData.m_spreadDistance + m_clusterSize));

		// Only perform "replanting" when moved more than one unit.
		//if ((eye - m_eye).length() >= 1.0f)
		{
			//m_eye = eye;

			for (AlignedVector< Cluster >::iterator i = m_clusters.begin(); i != m_clusters.end(); ++i)
			{
				i->distance = (i->center - eye).length();

				bool visible = i->visible;

				i->visible = (viewFrustum.inside(view * i->center, Scalar(m_clusterSize)) != Frustum::IrOutside);
				if (!i->visible)
					continue;

				if (i->visible && visible)
					continue;

				RandomGeometry random(int32_t(i->center.x() * 919.0f + i->center.z() * 463.0f));
				for (int32_t j = i->from; j < i->to; ++j)
				{
					float dx = (random.nextFloat() * 2.0f - 1.0f) * m_clusterSize;
					float dz = (random.nextFloat() * 2.0f - 1.0f) * m_clusterSize;

					float px = i->center.x() + dx;
					float pz = i->center.z() + dz;

					float py = terrain->getHeightfield()->getWorldHeight(px, pz);

					m_instances[j].position = Vector4(px, py, pz, 0.0f);
					m_instances[j].rotation = Quaternion::fromEulerAngles(random.nextFloat() * TWO_PI, (random.nextFloat() * 2.0f - 1.0f) * deg2rad(10.0f), 0.0f);
					m_instances[j].scale = random.nextFloat() * i->scale + (1.0f - i->scale);
				}
			}
		}
	}

	render::RenderContext* renderContext = worldContext.getRenderContext();

	// Expose some more shader parameters, such as terrain color etc.
	render::ProgramParameters* extraParameters = renderContext->alloc< render::ProgramParameters >();
	extraParameters->beginParameters(renderContext);
	extraParameters->setTextureParameter(s_handleNormals, terrain->getNormalMap());
	extraParameters->setTextureParameter(s_handleHeightfield, terrain->getHeightMap());
	extraParameters->setTextureParameter(s_handleSurface, terrainEntity.getSurfaceCache()->getBaseTexture());
	extraParameters->setVectorParameter(s_handleWorldExtent, terrain->getHeightfield()->getWorldExtent());
	extraParameters->setVectorParameter(s_handleEye, eye);
	extraParameters->setFloatParameter(s_handleMaxDistance, m_layerData.m_spreadDistance + m_clusterSize);
	extraParameters->endParameters(renderContext);

	AlignedVector< mesh::InstanceMesh::instance_distance_t > instanceData(mesh::InstanceMesh::MaxInstanceCount);
	uint32_t plantCount = 0;

	for (AlignedVector< Cluster >::const_iterator i = m_clusters.begin(); i != m_clusters.end(); ++i)
	{
		if (!i->visible)
			continue;

		int32_t count = i->to - i->from;
		for (int32_t j = 0; j < count; )
		{
			int32_t batch = std::min< int32_t >(count - j, mesh::InstanceMesh::MaxInstanceCount);

			instanceData.resize(batch);
			for (int32_t k = 0; k < batch; ++k, ++j)
			{
				m_instances[j + i->from].position.storeAligned( instanceData[k].first.translation );
				m_instances[j + i->from].rotation.e.storeAligned( instanceData[k].first.rotation );
				
				instanceData[k].first.scale = m_instances[j + i->from].scale;

				instanceData[k].second = i->distance;
			}

			m_mesh->render(
				renderContext,
				worldRenderPass,
				instanceData,
				extraParameters
			);
		}
	}
}

void RubbleLayer::updatePatches(const TerrainEntity& terrainEntity)
{
	m_instances.resize(0);
	m_clusters.resize(0);

	const resource::Proxy< Terrain >& terrain = terrainEntity.getTerrain();
	const resource::Proxy< hf::Heightfield >& heightfield = terrain->getHeightfield();

	// Get set of materials which have undergrowth.
	StaticVector< uint8_t, 16 > um(16, 0);
	uint8_t maxMaterialIndex = 0;
	for (std::vector< RubbleLayerData::RubbleMesh >::const_iterator i = m_layerData.m_rubble.begin(); i != m_layerData.m_rubble.end(); ++i)
		um[i->material] = ++maxMaterialIndex;

	int32_t size = heightfield->getSize();
	Vector4 extentPerGrid = heightfield->getWorldExtent() / Scalar(size);

	m_clusterSize = (16.0f / 2.0f) * max< float >(extentPerGrid.x(), extentPerGrid.z());

	// Create clusters.
	RandomGeometry random;
	for (int32_t z = 0; z < size; z += 16)
	{
		for (int32_t x = 0; x < size; x += 16)
		{
			StaticVector< int32_t, 16 > cm(16, 0);
			int32_t totalDensity = 0;

			for (int32_t cz = 0; cz < 16; ++cz)
			{
				for (int32_t cx = 0; cx < 16; ++cx)
				{
					uint8_t material = heightfield->getGridMaterial(x + cx, z + cz);
					uint8_t index = um[material];
					if (index > 0)
					{
						cm[index - 1]++;
						totalDensity++;
					}
				}
			}

			if (totalDensity <= 0)
				continue;

			float wx, wz;
			heightfield->gridToWorld(x + 8, z + 8, wx, wz);

			float wy = heightfield->getWorldHeight(wx, wz);

			for (uint32_t i = 0; i < maxMaterialIndex; ++i)
			{
				if (cm[i] <= 0)
					continue;

				const RubbleLayerData::RubbleMesh* plantDef = 0;
				for (std::vector< RubbleLayerData::RubbleMesh >::const_iterator j = m_layerData.m_rubble.begin(); j != m_layerData.m_rubble.end(); ++j)
				{
					if (um[j->material] == i + 1)
					{
						plantDef = &(*j);
						break;
					}
				}
				if (!plantDef)
					continue;

				int32_t densityFactor = cm[i];

				int32_t density = (plantDef->density * densityFactor) / (16 * 16);
				if (density <= 4)
					continue;

				size_t from = m_instances.size();
				for (int32_t j = 0; j < density; ++j)
				{
					Instance instance;
					instance.position = Vector4::zero();
					instance.rotation = Quaternion::identity();
					instance.scale = 0.0f;
					m_instances.push_back(instance);
				}
				size_t to = m_instances.size();

				Cluster c;
				c.center = Vector4(wx, wy, wz, 1.0f);
				c.distance = std::numeric_limits< float >::max();
				c.visible = false;
				c.scale = plantDef->randomScaleAmount;
				c.from = from;
				c.to = to;
				m_clusters.push_back(c);
			}
		}
	}
}

	}
}
