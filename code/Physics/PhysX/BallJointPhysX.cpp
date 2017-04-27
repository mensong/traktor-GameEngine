/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Physics/PhysX/BallJointPhysX.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.BallJointPhysX", BallJointPhysX, BallJoint)

BallJointPhysX::BallJointPhysX(IWorldCallback* callback, physx::PxJoint* joint, Body* body1, Body* body2)
:	JointPhysX< BallJoint >(callback, joint, body1, body2)
{
}

void BallJointPhysX::setAnchor(const Vector4& anchor)
{
}

Vector4 BallJointPhysX::getAnchor() const
{
	return Vector4(0.0f, 0.0f, 0.0f, 1.0f);
}

	}
}
