/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Const.h"
#include "Core/Math/Float.h"
#include "Physics/BodyState.h"
#include "Physics/Joint.h"
#include "Physics/Mesh.h"
#include "Physics/Jolt/Conversion.h"
#include "Physics/Jolt/BodyJolt.h"

// Keep Jolt includes here, Jolt.h must be first.
#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>

namespace traktor::physics
{
	namespace
	{

JPH::Vec3 convert(const Vector4& v)
{
	return JPH::Vec3(v.x(), v.y(), v.z());
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.BodyJolt", BodyJolt, Body)

BodyJolt::BodyJolt(
	const wchar_t* const tag,
	JPH::PhysicsSystem* physicsSystem,
	JPH::Body* body
)
:	Body(tag)
,	m_physicsSystem(physicsSystem)
,	m_body(body)
{
}

void BodyJolt::destroy()
{
	Body::destroy();
}

void BodyJolt::setTransform(const Transform& transform)
{
	JPH::BodyInterface& bodyInterface = m_physicsSystem->GetBodyInterface();
	bodyInterface.SetPosition(m_body->GetID(), convert(transform.translation()), JPH::EActivation::Activate);
}

Transform BodyJolt::getTransform() const
{
	const JPH::Vec3 position = m_body->GetCenterOfMassPosition();
	return Transform(Vector4(position.GetX(), position.GetY(), position.GetZ(), 1.0f));
}

Transform BodyJolt::getCenterTransform() const
{
	const JPH::Vec3 position = m_body->GetCenterOfMassPosition();
	return Transform(Vector4(position.GetX(), position.GetY(), position.GetZ(), 1.0f));
}

void BodyJolt::setKinematic(bool kinematic)
{
}

bool BodyJolt::isStatic() const
{
	return m_body->IsStatic();
}

bool BodyJolt::isKinematic() const
{
	return m_body->IsKinematic();
}

void BodyJolt::setActive(bool active)
{
}

bool BodyJolt::isActive() const
{
	return m_body->IsActive();
}

void BodyJolt::setEnable(bool enable)
{
}

bool BodyJolt::isEnable() const
{
	return false;
}

void BodyJolt::reset()
{
}

void BodyJolt::setMass(float mass, const Vector4& inertiaTensor)
{
}

float BodyJolt::getInverseMass() const
{
	return 0.0f;
}

Matrix33 BodyJolt::getInertiaTensorInverseWorld() const
{
	return Matrix33::identity();
}

void BodyJolt::addForceAt(const Vector4& at, const Vector4& force, bool localSpace)
{
	T_FATAL_ASSERT(!localSpace);
	m_body->AddForce(convert(force), convert(at));
}

void BodyJolt::addTorque(const Vector4& torque, bool localSpace)
{
	T_FATAL_ASSERT(!localSpace);
	m_body->AddTorque(convert(torque));
}

void BodyJolt::addLinearImpulse(const Vector4& linearImpulse, bool localSpace)
{
	T_FATAL_ASSERT(!localSpace);
	m_body->AddImpulse(convert(linearImpulse));
}

void BodyJolt::addAngularImpulse(const Vector4& angularImpulse, bool localSpace)
{
	T_FATAL_ASSERT(!localSpace);
	m_body->AddAngularImpulse(convert(angularImpulse));
}

void BodyJolt::addImpulse(const Vector4& at, const Vector4& impulse, bool localSpace)
{
	T_FATAL_ASSERT(!localSpace);
	m_body->AddImpulse(convert(impulse), convert(at));
}

void BodyJolt::setLinearVelocity(const Vector4& linearVelocity)
{
}

Vector4 BodyJolt::getLinearVelocity() const
{
	return Vector4::zero();
}

void BodyJolt::setAngularVelocity(const Vector4& angularVelocity)
{
}

Vector4 BodyJolt::getAngularVelocity() const
{
	return Vector4::zero();
}

Vector4 BodyJolt::getVelocityAt(const Vector4& at, bool localSpace) const
{
	return Vector4::zero();
}

void BodyJolt::setState(const BodyState& state)
{
	setTransform(state.getTransform());
	setLinearVelocity(state.getLinearVelocity());
	setAngularVelocity(state.getAngularVelocity());
}

BodyState BodyJolt::getState() const
{
	BodyState state;
	state.setTransform(getTransform());
	state.setLinearVelocity(getLinearVelocity());
	state.setAngularVelocity(getAngularVelocity());
	return state;
}

void BodyJolt::integrate(float deltaTime)
{
}

}
