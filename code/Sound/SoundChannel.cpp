#include <cmath>
#include <cstring>
#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
#include "Core/Memory/Alloc.h"
#include "Core/Thread/Acquire.h"
#include "Sound/IFilter.h"
#include "Sound/ISoundBuffer.h"
#include "Sound/Sound.h"
#include "Sound/SoundChannel.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.SoundChannel", SoundChannel, Object)

SoundChannel::SoundChannel(uint32_t hwSampleRate, uint32_t hwFrameSamples)
:	m_hwSampleRate(hwSampleRate)
,	m_hwFrameSamples(hwFrameSamples)
,	m_time(0.0)
,	m_repeat(0)
,	m_volume(1.0f)
{
	std::memset(m_outputSamples, 0, sizeof(m_outputSamples));
	for (uint32_t i = 0; i < SbcMaxChannelCount; ++i)
		m_outputSamples[i] = static_cast< float* >(Alloc::acquireAlign(hwFrameSamples * sizeof(float), 16));
}

SoundChannel::~SoundChannel()
{
	for (uint32_t i = 0; i < sizeof_array(m_outputSamples); ++i)
		Alloc::freeAlign(m_outputSamples[i]);
}

void SoundChannel::setVolume(float volume)
{
	m_volume = volume;
}

void SoundChannel::setFilter(IFilter* filter)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_filterLock);
	m_filter = filter;
}

Ref< IFilter > SoundChannel::getFilter() const
{
	return m_filter;
}

bool SoundChannel::isPlaying() const
{
	// Assume we're playing if we have an attached sound.
	return bool(m_sound != 0);
}

void SoundChannel::stop()
{
	m_sound = 0;
}

void SoundChannel::playSound(Sound* sound, double time, uint32_t repeat)
{
	Ref< ISoundBuffer > soundBuffer = sound->getSoundBuffer();
	if (soundBuffer)
	{
		m_sound = sound;
		m_time = time;
		m_repeat = max< uint32_t >(repeat, 1U);
		m_cursor = soundBuffer->createCursor();
	}
}

bool SoundChannel::getBlock(double time, SoundBlock& outBlock)
{
	if (!m_sound)
		return false;

	Ref< ISoundBuffer > soundBuffer = m_sound->getSoundBuffer();
	T_ASSERT (soundBuffer);

	// Local sound time.
	double soundTime = time - m_time;
	soundTime = std::max(0.0, soundTime);

	// Move cursor to local time.
	m_cursor->setCursor(soundTime);

	// Request sound block from buffer.
	SoundBlock soundBlock = { { 0, 0 }, outBlock.samplesCount, 0, 0 };
	if (!soundBuffer->getBlock(m_cursor, soundBlock))
	{
		// No more blocks from sound buffer.
		if (--m_repeat > 0)
		{
			m_cursor->setCursor(0.0);
			if (!soundBuffer->getBlock(m_cursor, soundBlock))
			{
				m_sound = 0;
				return false;
			}
			m_time = time;
		}
		else
		{
			m_sound = 0;
			return false;
		}
	}

	// We might get a null block; does not indicate end of stream.
	if (!soundBlock.samplesCount || !soundBlock.sampleRate || !soundBlock.maxChannel)
		return false;

	// Apply filter on sound block.
	if (m_filter)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_filterLock);
		m_filter->apply(soundBlock);
	}

	// Normalize sound block to hardware sample rate.
	outBlock.samplesCount = (soundBlock.samplesCount * m_hwSampleRate) / soundBlock.sampleRate;
	outBlock.samplesCount = std::min(outBlock.samplesCount, m_hwFrameSamples);
	outBlock.sampleRate = m_hwSampleRate;
	outBlock.maxChannel = soundBlock.maxChannel;

	if (soundBlock.sampleRate != m_hwSampleRate)
	{
		for (uint32_t i = 0; i < SbcMaxChannelCount; ++i)
			outBlock.samples[i] = soundBlock.samples[i] ? m_outputSamples[i] : 0;

		for (uint32_t i = 0; i < outBlock.samplesCount; ++i)
		{
			uint32_t j = (i * soundBlock.sampleRate) / m_hwSampleRate;
			for (uint32_t k = 0; k < outBlock.maxChannel; ++k)
			{
				if (outBlock.samples[k])
					outBlock.samples[k][i] = soundBlock.samples[k][j] * m_volume;
			}
		}
	}
	else
	{
		if (abs(m_volume - 1.0f) < FUZZY_EPSILON)
		{
			for (uint32_t i = 0; i < SbcMaxChannelCount; ++i)
				outBlock.samples[i] = soundBlock.samples[i];
		}
		else
		{
			for (uint32_t i = 0; i < SbcMaxChannelCount; ++i)
				outBlock.samples[i] = soundBlock.samples[i] ? m_outputSamples[i] : 0;

			for (uint32_t i = 0; i < outBlock.maxChannel; ++i)
			{
				if (outBlock.samples[i])
				{
					for (uint32_t j = 0; j < outBlock.samplesCount; ++j)
						outBlock.samples[i][j] = soundBlock.samples[i][j] * m_volume;
				}
			}
		}
	}

	return true;
}

	}
}
