#include "Core/Math/Const.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Mesh/Instance/InstanceMesh.h"
#include "Resource/Member.h"
#include "Terrain/ForestLayer.h"
#include "Terrain/ForestLayerData.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.terrain.ForestLayerData", 0, ForestLayerData, ITerrainLayerData)

ForestLayerData::ForestLayerData()
:	m_material(0)
,	m_density(0.1f)
,	m_lod0distance(50.0f)
,	m_lod1distance(200.0f)
,	m_upness(0.8f)
,	m_randomScale(0.2f)
,	m_randomTilt(deg2rad(4.0f))
{
}

Ref< ITerrainLayer > ForestLayerData::createLayerInstance(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	const TerrainComponent& terrainComponent
) const
{
	Ref< ForestLayer > layer = new ForestLayer();
	if (layer->create(resourceManager, renderSystem, *this, terrainComponent))
		return layer;
	else
		return nullptr;
}

void ForestLayerData::serialize(ISerializer& s)
{
	s >> resource::Member< mesh::InstanceMesh >(L"lod0mesh", m_lod0mesh);
	s >> resource::Member< mesh::InstanceMesh >(L"lod1mesh", m_lod1mesh);
	s >> Member< uint8_t >(L"material", m_material);
	s >> Member< float >(L"density", m_density, AttributeRange(0.0f));
	s >> Member< float >(L"lod0distance", m_lod0distance, AttributeRange(0.0f) | AttributeUnit(AuMetres));
	s >> Member< float >(L"lod1distance", m_lod1distance, AttributeRange(0.0f) | AttributeUnit(AuMetres));
	s >> Member< float >(L"upness", m_upness, AttributeRange(0.0f, 1.0f) | AttributeUnit(AuPercent));
	s >> Member< float >(L"randomScale", m_randomScale, AttributeRange(0.0f, 1.0f) | AttributeUnit(AuPercent));
	s >> Member< float >(L"randomTilt", m_randomTilt, AttributeRange(0.0f, HALF_PI) | AttributeUnit(AuDegrees));
}

	}
}
