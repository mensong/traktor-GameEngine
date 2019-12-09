#pragma once

#include "Sound/ISoundBuffer.h"

namespace traktor
{
	namespace spark
	{

class Sound;

class SoundBuffer : public sound::ISoundBuffer
{
	T_RTTI_CLASS;

public:
	SoundBuffer(const Sound* sound);

	virtual Ref< sound::ISoundBufferCursor > createCursor() const override final;

	virtual bool getBlock(sound::ISoundBufferCursor* cursor, const sound::IAudioMixer* mixer, sound::SoundBlock& outBlock) const override final;

private:
	Ref< const Sound > m_sound;
};

	}
}

