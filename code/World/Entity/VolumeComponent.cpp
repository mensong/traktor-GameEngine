#include "World/Entity.h"
#include "World/Entity/VolumeComponent.h"
#include "World/Entity/VolumeComponentData.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.VolumeComponent", VolumeComponent, IEntityComponent)

VolumeComponent::VolumeComponent(const VolumeComponentData* data)
:	m_owner(nullptr)
,	m_data(data)
{
	for (const auto& volume : m_data->getVolumes())
		m_boundingBox.contain(volume);
}

void VolumeComponent::destroy()
{
}

void VolumeComponent::setOwner(Entity* owner)
{
	m_owner = owner;
}

void VolumeComponent::setTransform(const Transform& transform)
{
}

Aabb3 VolumeComponent::getBoundingBox() const
{
	return m_boundingBox;
}

void VolumeComponent::update(const UpdateParams& update)
{
}

bool VolumeComponent::inside(const Vector4& point) const
{
	Transform transform;
	if (!m_owner || !m_owner->getTransform(transform))
		return false;

	Vector4 p = transform.inverse() * point.xyz1();
	for (const auto& volume : m_data->getVolumes())
	{
		if (volume.inside(p))
			return true;
	}

	return false;
}

	}
}
