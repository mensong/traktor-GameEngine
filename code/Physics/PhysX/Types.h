/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_physics_physx_Types_H
#define traktor_physics_physx_Types_H

namespace physx
{

class PxRigidActor;
class PxJoint;

}

namespace traktor
{
	namespace physics
	{

class Body;
class Joint;

/*!
 * \ingroup PhysX
 */
struct IWorldCallback
{
	virtual void insertActor(physx::PxRigidActor* actor) = 0;

	virtual void removeActor(physx::PxRigidActor* actor) = 0;

	virtual void destroyBody(Body* owner, physx::PxRigidActor* actor) = 0;

	virtual void destroyJoint(Joint* owner, physx::PxJoint* joint) = 0;
};

	}
}

#endif	// traktor_physics_physx_Types_H
