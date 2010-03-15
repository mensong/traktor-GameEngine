#include <cstring>
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
#include "Core/Memory/Alloc.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Timer/Timer.h"
#include "Sound/ISoundDriver.h"
#include "Sound/ISoundMixer.h"
#include "Sound/Sound.h"
#include "Sound/SoundChannel.h"
#include "Sound/SoundMixer.h"
#include "Sound/SoundSystem.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.SoundSystem", SoundSystem, Object)

SoundSystem::SoundSystem(ISoundDriver* driver)
:	m_driver(driver)
,	m_mixer(0)
,	m_threadMixer(0)
,	m_threadSubmit(0)
,	m_samplesData(0)
,	m_samplesBlockHead(0)
,	m_samplesBlockTail(0)
,	m_time(0.0)
,	m_mixerThreadTime(0.0)
,	m_submitThreadTime(0.0)
{
}

bool SoundSystem::create(const SoundSystemCreateDesc& desc)
{
	T_ASSERT (m_driver);

	m_desc = desc;
	m_desc.driverDesc.frameSamples &= ~3U;

	// Create driver.
	if (!m_driver->create(m_desc.driverDesc, m_mixer))
		return false;

	// If driver didn't create an alternative sound mixer we create
	// a default mixer.
	if (!m_mixer)
		m_mixer = new SoundMixer();

	// Allocate samples.
	uint32_t samplesBlockCount = m_desc.driverDesc.mixerFrames * 2;
	uint32_t samplesPerBlock = m_desc.driverDesc.frameSamples * m_desc.driverDesc.hwChannels;

	m_samplesData = static_cast< float* >(Alloc::acquireAlign(
		samplesPerBlock * samplesBlockCount * sizeof(float),
		16
	));
	if (!m_samplesData)
		return false;

	for (uint32_t i = 0; i < samplesBlockCount; ++i)
		m_samplesBlocks.push_back(&m_samplesData[i * samplesPerBlock]);

	// Create mixer and submission threads.
	m_threadMixer = ThreadManager::getInstance().create(makeFunctor(this, &SoundSystem::threadMixer), L"Sound mixer", 1);
	if (!m_threadMixer)
	{
		m_driver->destroy();
		return false;
	}

	m_threadSubmit = ThreadManager::getInstance().create(makeFunctor(this, &SoundSystem::threadSubmit), L"Sound submit", 2);
	if (!m_threadSubmit)
	{
		m_driver->destroy();
		return false;
	}

	// Create virtual channels.
	m_channels.resize(desc.channels);
	for (uint32_t i = 0; i < desc.channels; ++i)
	{
		m_channels[i] = new SoundChannel(
			desc.driverDesc.sampleRate,
			desc.driverDesc.frameSamples
		);
	}

	// Reset global playback time.
	m_time = 0.0;

	// Start threads.
	m_threadMixer->start();
	m_threadSubmit->start(Thread::Above);

	return true;
}

void SoundSystem::destroy()
{
	T_ASSERT (m_driver);

	if (m_threadSubmit)
	{
		m_threadSubmit->stop();
		ThreadManager::getInstance().destroy(m_threadSubmit);
	}

	if (m_threadMixer)
	{
		m_threadMixer->stop();
		ThreadManager::getInstance().destroy(m_threadMixer);
	}

	m_mixer = 0;
	safeDestroy(m_driver);

	if (m_samplesData)
	{
		Alloc::freeAlign(m_samplesData);
		m_samplesData = 0;
	}
}

void SoundSystem::setCombineMatrix(float cm[SbcMaxChannelCount][SbcMaxChannelCount])
{
	std::memcpy(m_desc.cm, cm, sizeof(cm));
}

Ref< SoundChannel > SoundSystem::getChannel(uint32_t channelId)
{
	T_ASSERT (channelId < m_channels.size());
	return m_channels[channelId];
}

Ref< SoundChannel > SoundSystem::play(uint32_t channelId, Sound* sound, uint32_t repeat)
{
	T_ASSERT (channelId < m_channels.size());
	Acquire< Semaphore > lock(m_channelAttachLock);
	m_channels[channelId]->playSound(sound, m_time, repeat);
	return m_channels[channelId];
}

Ref< SoundChannel > SoundSystem::play(Sound* sound, bool wait, uint32_t repeat)
{
	for (;;)
	{
		// Allocate first idle channel.
		{
			Acquire< Semaphore > lock(m_channelAttachLock);
			for (RefArray< SoundChannel >::iterator i = m_channels.begin(); i != m_channels.end(); ++i)
			{
				if (!(*i)->isPlaying())
				{
					(*i)->playSound(sound, m_time, repeat);
					return *i;
				}
			}
		}

		if (!wait)
			break;

		// Yield calling thread; @fixme Notification when channel is free.
		ThreadManager::getInstance().getCurrentThread()->sleep(10);
	}
	return 0;
}

void SoundSystem::stop(uint32_t channelId)
{
	T_ASSERT (channelId < m_channels.size());
	Acquire< Semaphore > lock(m_channelAttachLock);
	m_channels[channelId]->stop();
}

void SoundSystem::stopAll()
{
	Acquire< Semaphore > lock(m_channelAttachLock);
	for (RefArray< SoundChannel >::iterator i = m_channels.begin(); i != m_channels.end(); ++i)
		(*i)->stop();
}

double SoundSystem::getTime() const
{
	return m_time;
}

void SoundSystem::getThreadPerformances(double& outMixerTime, double& outSubmitTime) const
{
	outMixerTime = m_mixerThreadTime;
	outSubmitTime = m_submitThreadTime;
}

void SoundSystem::threadMixer()
{
	SoundBlock requestBlock;
	SoundBlock frameBlock;
	Timer timerMixer;

	timerMixer.start();

	while (!m_threadMixer->stopped())
	{
		// Wait until submission queue is below threshold.
		while (m_submitQueue.size() >= m_desc.driverDesc.mixerFrames && !m_threadMixer->stopped())
			m_submitConsumedEvent.wait(100);

		if (m_threadMixer->stopped())
			break;

		double startTime = timerMixer.getElapsedTime();

		// Allocate new frame block.
		float* samples = m_samplesBlocks[m_samplesBlockHead];
		m_samplesBlocks[m_samplesBlockHead] = 0;
		m_samplesBlockHead = (m_samplesBlockHead + 1) % m_samplesBlocks.size();
		T_ASSERT (samples);

		// Prepare new frame block.
		std::memset (samples, 0, m_desc.driverDesc.frameSamples * m_desc.driverDesc.hwChannels * sizeof(float));
		for (uint32_t i = 0; i < m_desc.driverDesc.hwChannels; ++i)
			frameBlock.samples[i] = samples + m_desc.driverDesc.frameSamples * i;
		frameBlock.samplesCount = m_desc.driverDesc.frameSamples;
		frameBlock.sampleRate = m_desc.driverDesc.sampleRate;
		frameBlock.maxChannel = m_desc.driverDesc.hwChannels;

		// Read blocks from channels.
		uint32_t channelsCount = uint32_t(m_channels.size());
		for (uint32_t i = 0; i < channelsCount; ++i)
		{
			requestBlock.samplesCount = m_desc.driverDesc.frameSamples;
			requestBlock.maxChannel = 0;

			// Temporarily lock channels as we don't want user to attach new sounds just yet.
			m_channelAttachLock.wait();
			bool got = m_channels[i]->getBlock(m_mixer, m_time, requestBlock);
			m_channelAttachLock.release();
			if (!got || !requestBlock.maxChannel)
				continue;

			T_ASSERT (requestBlock.sampleRate == m_desc.driverDesc.sampleRate);
			T_ASSERT (requestBlock.samplesCount == m_desc.driverDesc.frameSamples);

			// Final combine channels into hardware channels using "combine matrix".
			for (uint32_t j = 0; j < m_desc.driverDesc.hwChannels; ++j)
			{
				for (uint32_t k = 0; k < requestBlock.maxChannel; ++k)
				{
					float strength = m_desc.cm[j][k];
					if (requestBlock.samples[k] && abs(strength) >= FUZZY_EPSILON)
					{
						m_mixer->addMulConst(
							frameBlock.samples[j],
							requestBlock.samples[k],
							requestBlock.samplesCount,
							strength
						);
					}
				}
			}
		}

		m_time += double(m_desc.driverDesc.frameSamples) / m_desc.driverDesc.sampleRate;

		if (m_threadMixer->stopped())
			break;

		m_mixer->synchronize();

		m_submitQueueLock.wait();
		m_submitQueue.push_back(frameBlock);
		m_submitQueueLock.release();
		m_submitQueueEvent.broadcast();

		double endTime = timerMixer.getElapsedTime();

		m_mixerThreadTime = (endTime - startTime) * 0.1f + m_mixerThreadTime * 0.9f;
	}
}

void SoundSystem::threadSubmit()
{
	Timer timerSubmit;

	while (!m_threadSubmit->stopped())
	{
		m_submitQueueLock.wait();
		if (m_submitQueue.empty())
		{
			log::warning << L"Sound - submit thread starved, waiting for mixer to catch up" << Endl;
			while (m_submitQueue.size() < m_desc.driverDesc.mixerFrames && !m_threadSubmit->stopped())
			{
				m_submitQueueLock.release();
				m_submitQueueEvent.wait(100);
				m_submitQueueLock.wait();
			}
			if (m_threadSubmit->stopped())
				break;
		}

		// Submit block to driver.
		double startTime = timerSubmit.getElapsedTime();
		m_driver->submit(m_submitQueue.front());

		// Move block back into heap.
		T_ASSERT (m_samplesBlocks[m_samplesBlockTail] == 0);
		m_samplesBlocks[m_samplesBlockTail] = m_submitQueue.front().samples[0];
		m_samplesBlockTail = (m_samplesBlockTail + 1) % m_samplesBlocks.size();

		m_submitQueue.pop_front();
		m_submitQueueLock.release();

		m_submitConsumedEvent.broadcast();

		double endTime = timerSubmit.getElapsedTime();

		// Wait until driver is ready to play another block.
		m_driver->wait();

		m_submitThreadTime = (endTime - startTime) * 0.1f + m_submitThreadTime * 0.9f;
	}
}

	}
}
