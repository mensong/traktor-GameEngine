#pragma once

#include "Sound/ISoundResource.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

/*! \brief Stream sound resource.
 * \ingroup Sound
 */
class T_DLLCLASS StreamSoundResource : public ISoundResource
{
	T_RTTI_CLASS;

public:
	StreamSoundResource();

	virtual Ref< Sound > createSound(resource::IResourceManager* resourceManager, const db::Instance* resourceInstance) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	friend class SoundPipeline;

	const TypeInfo* m_decoderType;
	std::wstring m_category;
	float m_gain;
	float m_range;
	bool m_preload;
};

	}
}

