/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_world_VolumeComponent_H
#define traktor_world_VolumeComponent_H

#include "Core/Ref.h"
#include "World/IEntityComponent.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class Entity;
class VolumeComponentData;

/*! \brief
 * \ingroup World
 */
class T_DLLCLASS VolumeComponent : public IEntityComponent
{
	T_RTTI_CLASS;

public:
	VolumeComponent(const VolumeComponentData* data);

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void setOwner(Entity* owner) T_OVERRIDE T_FINAL;

	virtual void setTransform(const Transform& transform) T_OVERRIDE T_FINAL;

	virtual Aabb3 getBoundingBox() const T_OVERRIDE T_FINAL;

	virtual void update(const UpdateParams& update) T_OVERRIDE T_FINAL;

	bool inside(const Vector4& point) const;

private:
	Entity* m_owner;
	Ref< const VolumeComponentData > m_data;
	Aabb3 m_boundingBox;
};

	}
}

#endif	// traktor_world_VolumeComponent_H
