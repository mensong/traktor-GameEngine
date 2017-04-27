/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Spray/Sequence.h"
#include "Spray/SequenceInstance.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.Sequence", Sequence, Object)

Sequence::Sequence(const std::vector< Key >& keys)
:	m_keys(keys)
{
}

Ref< SequenceInstance > Sequence::createInstance() const
{
	return new SequenceInstance(this);
}

	}
}
