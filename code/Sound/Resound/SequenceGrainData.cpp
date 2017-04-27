/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Sound/Resound/IGrainFactory.h"
#include "Sound/Resound/SequenceGrain.h"
#include "Sound/Resound/SequenceGrainData.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.SequenceGrainData", 0, SequenceGrainData, IGrainData)

Ref< IGrain > SequenceGrainData::createInstance(IGrainFactory* grainFactory) const
{
	RefArray< IGrain > grains;

	grains.resize(m_grains.size());
	for (uint32_t i = 0; i < m_grains.size(); ++i)
	{
		grains[i] = grainFactory->createInstance(m_grains[i]);
		if (!grains[i])
			return 0;
	}

	return new SequenceGrain(grains);
}

void SequenceGrainData::serialize(ISerializer& s)
{
	s >> MemberRefArray< IGrainData >(L"grains", m_grains);
}

	}
}
