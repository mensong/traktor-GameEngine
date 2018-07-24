/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#pragma once

#include "Animation/Editor/SkeletonFormat.h"

namespace traktor
{
	namespace animation
	{

/*! \brief
 * \ingroup Animation
 */
class SkeletonFormatFbx : public SkeletonFormat
{
	T_RTTI_CLASS;

public:
	virtual void getExtensions(std::wstring& outDescription, std::vector< std::wstring >& outExtensions) const T_OVERRIDE T_FINAL;

	virtual bool supportFormat(const std::wstring& extension) const T_OVERRIDE T_FINAL;

	virtual Ref< Skeleton > read(IStream* stream, const Vector4& offset, float scale, float radius, bool invertX, bool invertZ) const T_OVERRIDE T_FINAL;
};

	}
}

