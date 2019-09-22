#include "World/Entity.h"
#include "World/Entity/ProbeComponent.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.ProbeComponent", ProbeComponent, IEntityComponent)

ProbeComponent::ProbeComponent(
	const resource::Proxy< render::ICubeTexture >& texture,
	float intensity,
	bool local,
	const Aabb3& volume
)
:	m_owner(nullptr)
,	m_texture(texture)
,	m_intensity(intensity)
,	m_local(local)
,	m_volume(volume)
,	m_dirty(true)
{
}

void ProbeComponent::destroy()
{
}

void ProbeComponent::setOwner(Entity* owner)
{
	m_owner = owner;
	m_dirty = true;
}

void ProbeComponent::update(const UpdateParams& update)
{
}

void ProbeComponent::setTransform(const Transform& transform)
{
	m_dirty = true;
}

Aabb3 ProbeComponent::getBoundingBox() const
{
	return m_volume;
}

Transform ProbeComponent::getTransform() const
{
	Transform transform;
	m_owner->getTransform(transform);
	return transform;
}

	}
}
