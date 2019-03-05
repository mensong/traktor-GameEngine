#include "Amalgam/IEnvironment.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Flash/Movie.h"
#include "Flash/Runtime/FlashLayer.h"
#include "Flash/Runtime/FlashLayerData.h"
#include "Render/ImageProcess/ImageProcessSettings.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.flash.FlashLayerData", 0, FlashLayerData, amalgam::LayerData)

FlashLayerData::FlashLayerData()
:	m_clearBackground(false)
,	m_enableShapeCache(false)
,	m_enableDirtyRegions(false)
,	m_enableSound(true)
,	m_contextSize(1)
{
}

Ref< amalgam::Layer > FlashLayerData::createInstance(amalgam::Stage* stage, amalgam::IEnvironment* environment) const
{
	resource::IResourceManager* resourceManager = environment->getResource()->getResourceManager();
	resource::Proxy< Movie > movie;
	resource::Proxy< render::ImageProcessSettings > imageProcess;

	// Bind proxies to resource manager.
	if (!resourceManager->bind(m_movie, movie))
		return nullptr;

	// Bind external movies.
	std::map< std::wstring, resource::Proxy< Movie > > externalMovies;
	for (std::map< std::wstring, resource::Id< Movie > >::const_iterator i = m_externalMovies.begin(); i != m_externalMovies.end(); ++i)
	{
		if (!resourceManager->bind(i->second, externalMovies[i->first]))
			return nullptr;
	}

	// Bind optional post processing.
	if (m_imageProcess)
	{
		if (!resourceManager->bind(m_imageProcess, imageProcess))
			return nullptr;
	}

	// Create layer instance.
	return new FlashLayer(
		stage,
		m_name,
		m_permitTransition,
		environment,
		movie,
		externalMovies,
		imageProcess,
		m_clearBackground,
		m_enableShapeCache,
		m_enableDirtyRegions,
		m_enableSound,
		m_contextSize
	);
}

void FlashLayerData::serialize(ISerializer& s)
{
	LayerData::serialize(s);

	s >> resource::Member< Movie >(L"movie", m_movie);

	s >> MemberStlMap<
		std::wstring,
		resource::Id< Movie >,
		MemberStlPair<
			std::wstring,
			resource::Id< Movie >,
			Member< std::wstring >,
			resource::Member< Movie >
		>
	>(L"externalMovies", m_externalMovies);

	s >> resource::Member< render::ImageProcessSettings >(L"imageProcess", m_imageProcess);
	s >> Member< bool >(L"clearBackground", m_clearBackground);
	s >> Member< bool >(L"enableSound", m_enableSound);
	s >> Member< bool >(L"enableShapeCache", m_enableShapeCache);
	s >> Member< bool >(L"enableDirtyRegions", m_enableDirtyRegions);
	s >> Member< uint32_t >(L"contextSize", m_contextSize);
}

	}
}
