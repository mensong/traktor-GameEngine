/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Heightfield/HeightfieldResource.h"

namespace traktor
{
	namespace hf
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.hf.HeightfieldResource", 1, HeightfieldResource, ISerializable)

HeightfieldResource::HeightfieldResource()
:	m_worldExtent(0.0f, 0.0f, 0.0f, 0.0f)
{
}

void HeightfieldResource::serialize(ISerializer& s)
{
	T_ASSERT (s.getVersion< HeightfieldResource >() >= 1);
	s >> Member< Vector4 >(L"worldExtent", m_worldExtent);
}

	}
}
