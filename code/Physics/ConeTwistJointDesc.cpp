#include "Physics/ConeTwistJointDesc.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Math/Const.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.physics.ConeTwistJointDesc", ConeTwistJointDesc, JointDesc)

ConeTwistJointDesc::ConeTwistJointDesc()
:	m_anchor(0.0f, 0.0f, 0.0f, 1.0f)
,	m_coneAxis(1.0f, 0.0f, 0.0f, 0.0f)
,	m_twistAxis(0.0f, 0.0f, 1.0f, 0.0f)
,	m_coneAngle1(0.0f)
,	m_coneAngle2(0.0f)
,	m_twistAngle(0.0f)
{
}

void ConeTwistJointDesc::setAnchor(const Vector4& anchor)
{
	m_anchor = anchor;
}

void ConeTwistJointDesc::setConeAxis(const Vector4& coneAxis)
{
	m_coneAxis = coneAxis;
}

void ConeTwistJointDesc::setTwistAxis(const Vector4& twistAxis)
{
	m_twistAxis = twistAxis;
}

void ConeTwistJointDesc::setConeAngles(float coneAngle1, float coneAngle2)
{
	m_coneAngle1 = coneAngle1;
	m_coneAngle2 = coneAngle2;
}

void ConeTwistJointDesc::setTwistAngle(float twistAngle)
{
	m_twistAngle = twistAngle;
}

bool ConeTwistJointDesc::serialize(Serializer& s)
{
	s >> Member< Vector4 >(L"anchor", m_anchor);
	s >> Member< Vector4 >(L"coneAxis", m_coneAxis);
	s >> Member< Vector4 >(L"twistAxis", m_twistAxis);
	s >> Member< float >(L"coneAngle1", m_coneAngle1, -PI, PI);
	s >> Member< float >(L"coneAngle2", m_coneAngle2, -PI, PI);
	s >> Member< float >(L"twistAngle", m_twistAngle, -PI, PI);
	return true;
}

	}
}
