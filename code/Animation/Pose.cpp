#include "Animation/Pose.h"
#include "Core/Serialization/AttributeAngles.h"
#include "Core/Serialization/AttributeDirection.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.Pose", 0, Pose, ISerializable)

void Pose::setJointOffset(uint32_t jointIndex, const Vector4& jointOffset)
{
	Joint& joint = getEditJoint(jointIndex);
	joint.offset = jointOffset;
}

Vector4 Pose::getJointOffset(uint32_t jointIndex) const
{
	const Joint* joint = getJoint(jointIndex);
	return joint ? joint->offset : Vector4::zero();
}

void Pose::setJointOrientation(uint32_t jointIndex, const Vector4& jointOrientation)
{
	Joint& joint = getEditJoint(jointIndex);
	joint.orientation = jointOrientation;
}

Vector4 Pose::getJointOrientation(uint32_t jointIndex) const
{
	const Joint* joint = getJoint(jointIndex);
	return joint ? joint->orientation : Vector4::zero();
}

void Pose::getIndexMask(BitSet& outIndices) const
{
	for (AlignedVector< Joint >::const_iterator i = m_joints.begin(); i != m_joints.end(); ++i)
		outIndices.set(i->index);
}

const Pose::Joint* Pose::getJoint(uint32_t jointIndex) const
{
	uint32_t s = 0;
	uint32_t e = uint32_t(m_joints.size());

	while (s < e)
	{
		uint32_t m = s + (e - s) / 2;
		if (jointIndex == m_joints[m].index)
			return &m_joints[m];
		else if (jointIndex < m_joints[m].index)
			e = m;
		else if (jointIndex > m_joints[m].index)
			s = m + 1;
	}

	return 0;
}

Pose::Joint& Pose::getEditJoint(uint32_t jointIndex)
{
	uint32_t s = 0;
	uint32_t e = uint32_t(m_joints.size());

	while (s < e)
	{
		uint32_t m = s + (e - s) / 2;
		if (jointIndex == m_joints[m].index)
			return m_joints[m];
		else if (jointIndex < m_joints[m].index)
			e = m;
		else if (jointIndex > m_joints[m].index)
			s = m + 1;
	}

	return *m_joints.insert(m_joints.begin() + s, Joint(jointIndex));
}

bool Pose::serialize(ISerializer& s)
{
	return s >> MemberAlignedVector< Joint, MemberComposite< Joint > >(L"joints", m_joints);
}

bool Pose::Joint::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"index", index);
	s >> Member< Vector4 >(L"offset", offset, AttributeDirection());
	s >> Member< Vector4 >(L"orientation", orientation, AttributeAngles());
	return true;
}

	}
}
