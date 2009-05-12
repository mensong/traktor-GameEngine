#ifndef traktor_sound_SoundDriverAlsa_H
#define traktor_sound_SoundDriverAlsa_H

#include "Sound/SoundDriver.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_ALSA_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

struct _snd_pcm;
struct _snd_pcm_hw_params;
struct _snd_pcm_sw_params;

namespace traktor
{
	namespace sound
	{
	
class T_DLLCLASS SoundDriverAlsa : public SoundDriver
{
	T_RTTI_CLASS(SoundDriverAlsa)
	
public:
	SoundDriverAlsa();

	virtual ~SoundDriverAlsa();

	virtual bool create(const SoundDriverCreateDesc& desc);

	virtual void destroy();

	virtual void wait();

	virtual void submit(const SoundBlock& soundBlock);

private:
	SoundDriverCreateDesc m_desc;
	_snd_pcm* m_handle;
	_snd_pcm_hw_params* m_hw_params;
	_snd_pcm_sw_params* m_sw_params;
	int16_t* m_buffer;
	uint32_t m_bufferCount;
};
	
	}
}

#endif	// traktor_sound_SoundDriverAlsa_H
