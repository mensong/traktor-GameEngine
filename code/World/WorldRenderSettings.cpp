#include "World/WorldRenderSettings.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRef.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.world.WorldRenderSettings", WorldRenderSettings, Serializable)

WorldRenderSettings::WorldRenderSettings()
:	hdr(true)
,	viewNearZ(1.0f)
,	viewFarZ(100.0f)
,	clearColor(0, 0, 0, 255)
,	depthPassEnabled(true)
,	shadowsEnabled(false)
,	shadowCascadingSlices(4)
,	shadowCascadingLambda(0.75f)
,	shadowFarZ(50.0f)
,	shadowMapResolution(1024)
,	shadowMapBias(0.001f)
{
}

bool WorldRenderSettings::serialize(Serializer& s)
{
	s >> Member< bool >(L"hdr", hdr);
	s >> Member< float >(L"viewNearZ", viewNearZ);
	s >> Member< float >(L"viewFarZ", viewFarZ);
	s >> Member< Color >(L"clearColor", clearColor);
	s >> Member< bool >(L"depthPassEnabled", depthPassEnabled);
	s >> Member< bool >(L"shadowsEnabled", shadowsEnabled);
	s >> Member< int32_t >(L"shadowCascadingSlices", shadowCascadingSlices);
	s >> Member< float >(L"shadowCascadingLambda", shadowCascadingLambda);
	s >> Member< float >(L"shadowFarZ", shadowFarZ);
	s >> Member< int32_t >(L"shadowMapResolution", shadowMapResolution);
	s >> Member< float >(L"shadowMapBias", shadowMapBias);
	return true;
}

	}
}
