#ifndef traktor_animation_SkeletonFormatCollada_H
#define traktor_animation_SkeletonFormatCollada_H

#include "Animation/Editor/ISkeletonFormat.h"

namespace traktor
{
	namespace animation
	{

//class BvhDocument;

class SkeletonFormatCollada : public ISkeletonFormat
{
	T_RTTI_CLASS;

public:
//	Ref< Skeleton > create(const BvhDocument* document) const;

	virtual Ref< Skeleton > import(IStream* stream, const Vector4& offset, float radius, bool invertX, bool invertZ) const T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_animation_SkeletonFormatCollada_H
