#include "Terrain/TerrainEntityData.h"
#include "Terrain/Heightfield.h"
#include "Terrain/HeightfieldResource.h"
#include "Terrain/TerrainSurface.h"
#include "Render/Shader.h"
#include "Render/ShaderGraph.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.terrain.TerrainEntityData", TerrainEntityData, world::EntityData)

TerrainEntityData::TerrainEntityData()
:	m_patchLodDistance(1500.0f)
,	m_surfaceLodDistance(100.0f)
{
}

bool TerrainEntityData::serialize(Serializer& s)
{
	if (!EntityData::serialize(s))
		return false;

	s >> resource::Member< Heightfield, HeightfieldResource >(L"heightfield", m_heightfield);
	s >> resource::Member< render::Shader, render::ShaderGraph >(L"shader", m_shader);
	s >> MemberRef< TerrainSurface >(L"surface", m_surface);
	s >> Member< float >(L"patchLodDistance", m_patchLodDistance);
	s >> Member< float >(L"surfaceLodDistance", m_surfaceLodDistance);

	return true;
}

	}
}
