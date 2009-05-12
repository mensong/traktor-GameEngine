#include "Animation/PathEntity/PathEntityData.h"
#include "Animation/PathEntity/Path.h"
#include "World/Entity/EntityBuilder.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberEnum.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.animation.PathEntityData", PathEntityData, world::SpatialEntityData)

PathEntityData::PathEntityData()
:	m_timeMode(PathEntity::TmManual)
{
}

PathEntity* PathEntityData::createEntity(world::EntityBuilder* builder) const
{
	Ref< world::SpatialEntity > entity = dynamic_type_cast< world::SpatialEntity* >(builder->build(m_entityData));
	return gc_new< PathEntity >(
		cref(getTransform()),
		cref(m_path),
		m_timeMode,
		entity
	);
}

bool PathEntityData::serialize(Serializer& s)
{
	const MemberEnum< PathEntity::TimeMode >::Key c_TimeMode_Keys[] =
	{
		{ L"TmManual", PathEntity::TmManual },
		{ L"TmOnce", PathEntity::TmOnce },
		{ L"TmLoop", PathEntity::TmLoop },
		{ L"TmPingPong", PathEntity::TmPingPong },
		{ 0 }
	};

	if (!world::SpatialEntityData::serialize(s))
		return false;

	s >> MemberComposite< Path >(L"path", m_path);
	s >> MemberEnum< PathEntity::TimeMode >(L"timeMode", m_timeMode, c_TimeMode_Keys);
	s >> MemberRef< world::SpatialEntityData >(L"entityData", m_entityData);

	return true;
}

	}
}
