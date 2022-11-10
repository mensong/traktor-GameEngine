/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Sound/ISoundBuffer.h"
#include "Sound/Processor/GraphEvaluator.h"
#include "Sound/Processor/Nodes/Pitch.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

const ImmutableNode::InputPinDesc c_Pitch_i[] =
{
	{ L"Input", NptSignal, false },
	{ L"Adjust", NptScalar, false },
	{ 0 }
};

const ImmutableNode::OutputPinDesc c_Pitch_o[] =
{
	{ L"Output", NptSignal },
	{ 0 }
};

class PitchCursor : public RefCountImpl< ISoundBufferCursor >
{
public:
	virtual void setParameter(handle_t id, float parameter) override final {}

	virtual void disableRepeat() override final {}

	virtual void reset() override final {}
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.Pitch", 0, Pitch, ImmutableNode)

Pitch::Pitch()
:	ImmutableNode(c_Pitch_i, c_Pitch_o)
{
}

bool Pitch::bind(resource::IResourceManager* resourceManager)
{
	return true;
}

Ref< ISoundBufferCursor > Pitch::createCursor() const
{
	return new PitchCursor();
}

bool Pitch::getScalar(ISoundBufferCursor* cursor, const GraphEvaluator* evaluator, float& outScalar) const
{
	return false;
}

bool Pitch::getBlock(ISoundBufferCursor* cursor, const GraphEvaluator* evaluator, const IAudioMixer* mixer, SoundBlock& outBlock) const
{
	float adjust;
	if (!evaluator->evaluateScalar(getInputPin(1), adjust))
		return false;

	if (adjust <= 0.0f)
		return false;

	if (!evaluator->evaluateBlock(getInputPin(0), mixer, outBlock))
		return false;

	outBlock.sampleRate = uint32_t(outBlock.sampleRate * adjust + 0.5f);
	return true;
}

	}
}
