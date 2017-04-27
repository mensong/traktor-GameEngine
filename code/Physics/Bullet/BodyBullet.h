/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_physics_BodyBullet_H
#define traktor_physics_BodyBullet_H

#include "Physics/Body.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_BULLET_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

// Bullet forward declarations.
class btCollisionShape;
class btTypedConstraint;
class btDynamicsWorld;
class btRigidBody;

namespace traktor
{
	namespace physics
	{

struct IWorldCallback;

/*!
 * \ingroup Bullet
 */
class T_DLLCLASS BodyBullet : public Body
{
	T_RTTI_CLASS;

public:
	BodyBullet(
		const wchar_t* const tag,
		IWorldCallback* callback,
		btDynamicsWorld* dynamicsWorld,
		float timeScale,
		btRigidBody* body,
		btCollisionShape* shape,
		const Vector4& centerOfGravity,
		uint32_t collisionGroup,
		uint32_t collisionMask,
		int32_t material
	);

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void setTransform(const Transform& transform) T_OVERRIDE T_FINAL;

	virtual Transform getTransform() const T_OVERRIDE T_FINAL;

	virtual Transform getCenterTransform() const T_OVERRIDE T_FINAL;

	virtual void setKinematic(bool kinematic) T_OVERRIDE T_FINAL;

	virtual bool isStatic() const T_OVERRIDE T_FINAL;

	virtual bool isKinematic() const T_OVERRIDE T_FINAL;

	virtual void setActive(bool active) T_OVERRIDE T_FINAL;

	virtual bool isActive() const T_OVERRIDE T_FINAL;

	virtual void setEnable(bool enable) T_OVERRIDE T_FINAL;

	virtual bool isEnable() const T_OVERRIDE T_FINAL;

	virtual void reset() T_OVERRIDE T_FINAL;

	virtual void setMass(float mass, const Vector4& inertiaTensor) T_OVERRIDE T_FINAL;

	virtual float getInverseMass() const T_OVERRIDE T_FINAL;

	virtual Matrix33 getInertiaTensorInverseWorld() const T_OVERRIDE T_FINAL;

	virtual void addForceAt(const Vector4& at, const Vector4& force, bool localSpace) T_OVERRIDE T_FINAL;

	virtual void addTorque(const Vector4& torque, bool localSpace) T_OVERRIDE T_FINAL;

	virtual void addLinearImpulse(const Vector4& linearImpulse, bool localSpace) T_OVERRIDE T_FINAL;

	virtual void addAngularImpulse(const Vector4& angularImpulse, bool localSpace) T_OVERRIDE T_FINAL;

	virtual void addImpulse(const Vector4& at, const Vector4& impulse, bool localSpace) T_OVERRIDE T_FINAL;

	virtual void setLinearVelocity(const Vector4& linearVelocity) T_OVERRIDE T_FINAL;

	virtual Vector4 getLinearVelocity() const T_OVERRIDE T_FINAL;

	virtual void setAngularVelocity(const Vector4& angularVelocity) T_OVERRIDE T_FINAL;

	virtual Vector4 getAngularVelocity() const T_OVERRIDE T_FINAL;

	virtual Vector4 getVelocityAt(const Vector4& at, bool localSpace) const T_OVERRIDE T_FINAL;

	virtual bool setState(const BodyState& state) T_OVERRIDE T_FINAL;

	virtual BodyState getState() const T_OVERRIDE T_FINAL;

	virtual void integrate(float deltaTime) T_OVERRIDE T_FINAL;

	void addConstraint(btTypedConstraint* constraint);

	void removeConstraint(btTypedConstraint* constraint);

	Transform getBodyTransform() const;

	btDynamicsWorld* getBtDynamicsWorld() const { return m_dynamicsWorld; }

	btRigidBody* getBtRigidBody() const { return m_body; }

	btCollisionShape* getBtCollisionShape() const { return m_shape; }

	uint32_t getCollisionGroup() const { return m_collisionGroup; }

	uint32_t getCollisionMask() const { return m_collisionMask; }

	int32_t getMaterial() const { return m_material; }

	const std::vector< btTypedConstraint* >& getJoints() const { return m_constraints; }

private:
	IWorldCallback* m_callback;
	btDynamicsWorld* m_dynamicsWorld;
	float m_timeScale;
	btRigidBody* m_body;
	btCollisionShape* m_shape;
	Vector4 m_centerOfGravity;
	uint32_t m_collisionGroup;
	uint32_t m_collisionMask;
	int32_t m_material;
	std::vector< btTypedConstraint* > m_constraints;
	bool m_enable;
};

	}
}

#endif	// traktor_physics_BodyBullet_H
