/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Core/Math/Envelope.h"
#include "Sound/Resound/IGrain.h"

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

/*! \brief
 * \ingroup Sound
 */
class T_DLLCLASS EnvelopeGrain : public IGrain
{
	T_RTTI_CLASS;

public:
	struct Grain
	{
		Ref< IGrain > grain;
		float in;
		float out;
		float easeIn;
		float easeOut;
	};

	EnvelopeGrain(handle_t id, const std::vector< Grain >& grains, const float levels[3], float mid, float response);

	virtual Ref< ISoundBufferCursor > createCursor() const override final;

	virtual void updateCursor(ISoundBufferCursor* cursor) const override final;

	virtual const IGrain* getCurrentGrain(const ISoundBufferCursor* cursor) const override final;

	virtual void getActiveGrains(const ISoundBufferCursor* cursor, RefArray< const IGrain >& outActiveGrains) const override final;

	virtual bool getBlock(ISoundBufferCursor* cursor, const IAudioMixer* mixer, SoundBlock& outBlock) const override final;

private:
	handle_t m_id;
	std::vector< Grain > m_grains;
	float m_response;
	Envelope< float, HermiteEvaluator< float > > m_envelope;
};

	}
}

