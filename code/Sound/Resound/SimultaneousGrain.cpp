/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include "Core/Math/MathUtils.h"
#include "Core/Memory/Alloc.h"
#include "Sound/IAudioMixer.h"
#include "Sound/ISoundBuffer.h"
#include "Sound/Resound/SimultaneousGrain.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

const uint32_t c_outputSamplesBlockCount = 8;

struct SimultaneousGrainCursor : public RefCountImpl< ISoundBufferCursor >
{
	RefArray< ISoundBufferCursor > m_grainCursors;
	float* m_outputSamples[SbcMaxChannelCount];

	SimultaneousGrainCursor()
	{
		m_outputSamples[0] = 0;
	}

	virtual ~SimultaneousGrainCursor()
	{
		Alloc::freeAlign(m_outputSamples[0]);
	}

	virtual void setParameter(handle_t id, float parameter) override final
	{
		for (RefArray< ISoundBufferCursor >::iterator i = m_grainCursors.begin(); i != m_grainCursors.end(); ++i)
			(*i)->setParameter(id, parameter);
	}

	virtual void disableRepeat() override final
	{
		for (RefArray< ISoundBufferCursor >::iterator i = m_grainCursors.begin(); i != m_grainCursors.end(); ++i)
			(*i)->disableRepeat();
	}

	virtual void reset() override final
	{
		for (RefArray< ISoundBufferCursor >::iterator i = m_grainCursors.begin(); i != m_grainCursors.end(); ++i)
			(*i)->reset();
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.SimultaneousGrain", SimultaneousGrain, IGrain)

SimultaneousGrain::SimultaneousGrain(const RefArray< IGrain >& grains)
:	m_grains(grains)
{
}

Ref< ISoundBufferCursor > SimultaneousGrain::createCursor() const
{
	if (m_grains.empty())
		return 0;

	Ref< SimultaneousGrainCursor > cursor = new SimultaneousGrainCursor();
	for (RefArray< IGrain >::const_iterator i = m_grains.begin(); i != m_grains.end(); ++i)
	{
		Ref< ISoundBufferCursor > childCursor = (*i)->createCursor();
		if (!childCursor)
			return 0;

		cursor->m_grainCursors.push_back(childCursor);
	}

	const uint32_t outputSamplesCount = 1024/*hwFrameSamples*/ * c_outputSamplesBlockCount;
	const uint32_t outputSamplesSize = SbcMaxChannelCount * outputSamplesCount * sizeof(float);

	cursor->m_outputSamples[0] = static_cast< float* >(Alloc::acquireAlign(outputSamplesSize, 16, T_FILE_LINE));
	std::memset(cursor->m_outputSamples[0], 0, outputSamplesSize);

	for (uint32_t i = 1; i < SbcMaxChannelCount; ++i)
		cursor->m_outputSamples[i] = cursor->m_outputSamples[0] + outputSamplesCount * i;

	return cursor;
}

void SimultaneousGrain::updateCursor(ISoundBufferCursor* cursor) const
{
	SimultaneousGrainCursor* simultaneousCursor = static_cast< SimultaneousGrainCursor* >(cursor);
	for (uint32_t i = 0; i < m_grains.size(); ++i)
		m_grains[i]->updateCursor(simultaneousCursor->m_grainCursors[i]);
}

const IGrain* SimultaneousGrain::getCurrentGrain(const ISoundBufferCursor* cursor) const
{
	return this;
}

void SimultaneousGrain::getActiveGrains(const ISoundBufferCursor* cursor, RefArray< const IGrain >& outActiveGrains) const
{
	const SimultaneousGrainCursor* simultaneousCursor = static_cast< const SimultaneousGrainCursor* >(cursor);

	outActiveGrains.push_back(this);

	for (uint32_t i = 0; i < m_grains.size(); ++i)
		m_grains[i]->getActiveGrains(simultaneousCursor->m_grainCursors[i], outActiveGrains);
}

bool SimultaneousGrain::getBlock(ISoundBufferCursor* cursor, const IAudioMixer* mixer, SoundBlock& outBlock) const
{
	SimultaneousGrainCursor* simultaneousCursor = static_cast< SimultaneousGrainCursor* >(cursor);

	bool playing = false;
	for (uint32_t i = 0; i < m_grains.size(); ++i)
	{
		SoundBlock soundBlock = { { 0 }, outBlock.samplesCount, 0, 0 };

		if (m_grains[i]->getBlock(
			simultaneousCursor->m_grainCursors[i],
			mixer,
			soundBlock
		))
		{
			outBlock.sampleRate = max(outBlock.sampleRate, soundBlock.sampleRate);
			outBlock.samplesCount = max(outBlock.samplesCount, soundBlock.samplesCount);
			outBlock.maxChannel = max(outBlock.maxChannel, soundBlock.maxChannel);

			for (uint32_t j = 0; j < soundBlock.maxChannel; ++j)
			{
				if (soundBlock.samples[j])
				{
					if (outBlock.samples[j])
					{
						mixer->addMulConst(
							outBlock.samples[j],
							soundBlock.samples[j],
							soundBlock.samplesCount,
							1.0f
						);
					}
					else
					{
						outBlock.samples[j] = simultaneousCursor->m_outputSamples[j];
						mixer->mulConst(
							outBlock.samples[j],
							soundBlock.samples[j],
							soundBlock.samplesCount,
							1.0f
						);
					}
				}
			}

			playing = true;
		}
	}

	return playing;
}

	}
}
