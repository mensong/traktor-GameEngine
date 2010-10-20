#include "Spray/Sources/PointSetSource.h"
#include "Spray/EmitterUpdateContext.h"
#include "Spray/EmitterInstance.h"
#include "Spray/PointSet.h"
#include "Spray/PointSetResource.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComposite.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.PointSetSource", 0, PointSetSource, Source)

PointSetSource::PointSetSource()
:	m_offset(0.0f, 0.0f, 0.0f, 0.0f)
,	m_velocity(0.0f, 0.0f)
,	m_orientation(0.0f, 2.0f * PI)
,	m_angularVelocity(0.0f, 0.0f)
,	m_age(1.0f, 1.0f)
,	m_mass(1.0f, 1.0f)
,	m_size(1.0f, 1.0f)
{
}

bool PointSetSource::bind(resource::IResourceManager* resourceManager)
{
	return resourceManager->bind(m_pointSet);
}

void PointSetSource::emit(
	EmitterUpdateContext& context,
	const Transform& transform,
	uint32_t emitCount,
	EmitterInstance& emitterInstance
) const
{
	if (!m_pointSet.validate())
		return;

	const AlignedVector< PointSet::Point >& points = m_pointSet->get();
	if (points.empty())
		return;

	Point* point = emitterInstance.addPoints(points.size());

	for (uint32_t i = 0; i < uint32_t(points.size()); ++i)
	{
		Vector4 position = transform * (points[i].position + m_offset).xyz1();
		Vector4 normal = transform * points[i].normal.xyz0();

		point->position = position;
		point->velocity = normal * Scalar(m_velocity.random(context.random));
		point->orientation = m_orientation.random(context.random);
		point->angularVelocity = m_angularVelocity.random(context.random);
		point->color = points[i].color;
		point->age = 0.0f;
		point->maxAge = m_age.random(context.random);
		point->inverseMass = 1.0f / (m_mass.random(context.random));
		point->size = m_size.random(context.random);
		point->random = context.random.nextFloat();
		
		++point;
	}
}

bool PointSetSource::serialize(ISerializer& s)
{
	if (!Source::serialize(s))
		return false;

	s >> resource::Member< PointSet, PointSetResource >(L"pointSet", m_pointSet);
	s >> MemberComposite< Range< float > >(L"velocity", m_velocity);
	s >> MemberComposite< Range< float > >(L"orientation", m_orientation);
	s >> MemberComposite< Range< float > >(L"angularVelocity", m_angularVelocity);
	s >> MemberComposite< Range< float > >(L"age", m_age);
	s >> MemberComposite< Range< float > >(L"mass", m_mass);
	s >> MemberComposite< Range< float > >(L"size", m_size);
	s >> Member< Vector4 >(L"offset", m_offset);

	return true;
}

	}
}
