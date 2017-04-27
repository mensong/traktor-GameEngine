/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_sound_BankBuffer_H
#define traktor_sound_BankBuffer_H

#include "Core/RefArray.h"
#include "Core/Thread/Semaphore.h"
#include "Sound/ISoundBuffer.h"

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

class IGrain;

class T_DLLCLASS BankBuffer : public ISoundBuffer
{
	T_RTTI_CLASS;

public:
	BankBuffer(const RefArray< IGrain >& grains);

	void updateCursor(ISoundBufferCursor* cursor) const;

	const IGrain* getCurrentGrain(const ISoundBufferCursor* cursor) const;

	void getActiveGrains(const ISoundBufferCursor* cursor, RefArray< const IGrain >& outActiveGrains) const;

	virtual Ref< ISoundBufferCursor > createCursor() const T_OVERRIDE T_FINAL;

	virtual bool getBlock(ISoundBufferCursor* cursor, const ISoundMixer* mixer, SoundBlock& outBlock) const T_OVERRIDE T_FINAL;

private:
	RefArray< IGrain > m_grains;
	mutable Semaphore m_lock;
};

	}
}

#endif	// traktor_sound_BankBuffer_H
