#include <algorithm>
#include <NxPhysics.h>
#include <NxCooking.h>
#include <NxStream.h>
#include "Physics/PhysX/PhysicsManagerPhysX.h"
#include "Physics/PhysX/StaticBodyPhysX.h"
#include "Physics/PhysX/DynamicBodyPhysX.h"
#include "Physics/PhysX/BallJointPhysX.h"
#include "Physics/PhysX/ConeTwistJointPhysX.h"
#include "Physics/PhysX/NxStreamWrapper.h"
#include "Physics/PhysX/Conversion.h"
#include "Physics/BoxShapeDesc.h"
#include "Physics/CapsuleShapeDesc.h"
#include "Physics/CylinderShapeDesc.h"
#include "Physics/MeshShapeDesc.h"
#include "Physics/SphereShapeDesc.h"
#include "Physics/HeightfieldShapeDesc.h"
#include "Physics/StaticBodyDesc.h"
#include "Physics/DynamicBodyDesc.h"
#include "Physics/BallJointDesc.h"
#include "Physics/ConeTwistJointDesc.h"
#include "Physics/HingeJointDesc.h"
#include "Physics/Hinge2JointDesc.h"
#include "Physics/Mesh.h"
#include "Physics/Heightfield.h"
#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace physics
	{
		namespace
		{

const NxCollisionGroup c_defaultGroup = 0;
const NxCollisionGroup c_ignoreBodyGroup = 1;

void setActorGroup(NxActor *actor, NxCollisionGroup group)
{
	NxU32 count = actor->getNbShapes();
	NxShape* const* shapes = actor->getShapes();
	while (count--)
		shapes[count]->setGroup(group);
}

		}

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.physics.PhysicsManagerPhysX", PhysicsManagerPhysX, PhysicsManager)

PhysicsManagerPhysX::PhysicsManagerPhysX()
:	m_simulationDeltaTime(0.0f)
,	m_sdk(0)
,	m_scene(0)
{
}

PhysicsManagerPhysX::~PhysicsManagerPhysX()
{
	destroy();
}

bool PhysicsManagerPhysX::create(float simulationDeltaTime)
{
	NxSDKCreateError errorCode = NXCE_NO_ERROR;
	NxPhysicsSDKDesc sdkDesc;
	NxSceneDesc sceneDesc;

	m_sdk = NxCreatePhysicsSDK(NX_PHYSICS_SDK_VERSION, NULL, 0, sdkDesc, &errorCode);
	if (!m_sdk) 
	{
		log::error << L"Unable to initialize the PhysX SDK" << Endl;
		return false;
	}

	m_sdk->setParameter(NX_SKIN_WIDTH, 0.01f);

	sceneDesc.gravity = NxVec3(0.0f, -9.81f, 0.0f);
	m_scene = m_sdk->createScene(sceneDesc);
	if (!m_scene) 
	{
		log::error << L"Unable to create a PhysX scene" << Endl;
		return false;
	}

	// Adjust default material.
	NxMaterial* defaultMaterial = m_scene->getMaterialFromIndex(0);
	T_ASSERT (defaultMaterial);

	defaultMaterial->setRestitution(0.05f);
	defaultMaterial->setStaticFriction(0.1f);
	defaultMaterial->setDynamicFriction(0.75f);

	m_simulationDeltaTime = simulationDeltaTime;
	return true;
}

void PhysicsManagerPhysX::destroy()
{
	if (m_scene)
	{
		T_ASSERT (m_sdk);
		m_sdk->releaseScene(*m_scene);
		m_scene = 0;
	}
	if (m_sdk)
	{
		NxReleasePhysicsSDK(m_sdk);
		m_sdk = 0;
	}
}

void PhysicsManagerPhysX::setGravity(const Vector4& gravity)
{
	m_scene->setGravity(toNxVec3(gravity));
}

Body* PhysicsManagerPhysX::createBody(const BodyDesc* desc)
{
	if (!desc)
		return 0;

	const ShapeDesc* shapeDesc = desc->getShape();
	if (!shapeDesc)
	{
		log::error << L"Unable to create body, no shape defined" << Endl;
		return 0;
	}

	NxShapeDesc* actorShapeDesc = 0;
	NxVec3 actorShapeOffset(0.0f, 0.0f, 0.0f);

	if (const BoxShapeDesc* boxShape = dynamic_type_cast< const BoxShapeDesc* >(shapeDesc))
	{
		NxBoxShapeDesc* boxShapeDesc = new NxBoxShapeDesc();
		boxShapeDesc->dimensions = toNxVec3(boxShape->getExtent());
		actorShapeDesc = boxShapeDesc;
	}
	else if (const CapsuleShapeDesc* capsuleShape = dynamic_type_cast< const CapsuleShapeDesc* >(shapeDesc))
	{
		float radius = capsuleShape->getRadius();
		float length = capsuleShape->getLength() - radius * 2.0f;

		NxCapsuleShapeDesc* capsuleShapeDesc = new NxCapsuleShapeDesc();
		capsuleShapeDesc->height = length;
		capsuleShapeDesc->radius = radius;

		actorShapeDesc = capsuleShapeDesc;
	}
	else if (const CylinderShapeDesc* cylinderShape = dynamic_type_cast< const CylinderShapeDesc* >(shapeDesc))
	{
		float radius = cylinderShape->getRadius();
		float length = cylinderShape->getLength() - radius * 2.0f;

		// As PhysX doesn't support cylinders we approximate with a capsule instead.
		NxCapsuleShapeDesc* capsuleShapeDesc = new NxCapsuleShapeDesc();
		capsuleShapeDesc->height = length;
		capsuleShapeDesc->radius = radius;

		actorShapeDesc = capsuleShapeDesc;
	}
	else if (const MeshShapeDesc* meshShape = dynamic_type_cast< const MeshShapeDesc* >(shapeDesc))
	{
		resource::Proxy< Mesh > mesh = meshShape->getMesh();
		if (!mesh.validate())
		{
			log::error << L"Unable to load mesh resource" << Endl;
			return 0;
		}

		const AlignedVector< Vector4 >& vertices = mesh->getVertices();
		const std::vector< Mesh::Triangle >& shapeTriangles = mesh->getShapeTriangles();
		const std::vector< Mesh::Triangle >& hullTriangles = mesh->getHullTriangles();

		if (is_a< StaticBodyDesc >(desc))
		{
			NxTriangleMeshDesc meshDesc;
			meshDesc.numVertices = NxU32(vertices.size());
			meshDesc.numTriangles = NxU32(shapeTriangles.size());
			meshDesc.pointStrideBytes = sizeof(Vector4);
			meshDesc.triangleStrideBytes = sizeof(Mesh::Triangle);
			meshDesc.points = &vertices[0];
			meshDesc.triangles = &shapeTriangles[0];
			meshDesc.flags = NX_MF_FLIPNORMALS;

			std::vector< uint8_t > buffer;

			NxCookingInterface* cooking = NxGetCookingLib(NX_PHYSICS_SDK_VERSION);
			T_ASSERT (cooking);

			DynamicMemoryStream ws(buffer, false, true);
			NxStreamWrapper nxws(ws);

			cooking->NxInitCooking();
			cooking->NxCookTriangleMesh(meshDesc, nxws);

			DynamicMemoryStream rs(buffer, true, false);

			NxTriangleMeshShapeDesc* meshShapeDesc = new NxTriangleMeshShapeDesc();
			meshShapeDesc->meshData = m_sdk->createTriangleMesh(NxStreamWrapper(rs));
			T_ASSERT (meshShapeDesc->meshData);

			actorShapeDesc = meshShapeDesc;
		}
		else	// Assume dynamic body.
		{
			NxConvexMeshDesc meshDesc;
			meshDesc.numVertices = NxU32(vertices.size());
			meshDesc.pointStrideBytes = sizeof(Vector4);
			meshDesc.points = &vertices[0];
			meshDesc.flags = NX_CF_COMPUTE_CONVEX;

			std::vector< uint8_t > buffer;

			NxCookingInterface* cooking = NxGetCookingLib(NX_PHYSICS_SDK_VERSION);
			T_ASSERT (cooking);

			DynamicMemoryStream ws(buffer, false, true);
			NxStreamWrapper nxws(ws);

			cooking->NxInitCooking();
			cooking->NxCookConvexMesh(meshDesc, nxws);

			DynamicMemoryStream rs(buffer, true, false);
			NxStreamWrapper nxrs(rs);

			NxConvexShapeDesc* meshShapeDesc = new NxConvexShapeDesc();
			meshShapeDesc->meshData = m_sdk->createConvexMesh(nxrs);
			T_ASSERT (meshShapeDesc->meshData);

			actorShapeDesc = meshShapeDesc;
		}
	}
	else if (const SphereShapeDesc* sphereShape = dynamic_type_cast< const SphereShapeDesc* >(shapeDesc))
	{
		NxSphereShapeDesc* sphereShapeDesc = new NxSphereShapeDesc();
		sphereShapeDesc->radius = sphereShape->getRadius();
		actorShapeDesc = sphereShapeDesc;
	}
	else if (const HeightfieldShapeDesc* heightfieldShape = dynamic_type_cast< const HeightfieldShapeDesc* >(shapeDesc))
	{
		resource::Proxy< Heightfield > heightfield = heightfieldShape->getHeightfield();
		if (!heightfield.validate())
		{
			log::error << L"Unable to load heightfield resource" << Endl;
			return 0;
		}

		const float* heights = heightfield->getHeights();

		NxHeightFieldDesc heightFieldDesc;
		heightFieldDesc.nbColumns = heightfield->getSize();
		heightFieldDesc.nbRows = heightfield->getSize();
		heightFieldDesc.convexEdgeThreshold = 0;
		heightFieldDesc.samples = new NxU32 [heightfield->getSize() * heightfield->getSize()];
		heightFieldDesc.sampleStride = sizeof(NxU32);
		heightFieldDesc.verticalExtent = -1000;

		NxU8* ptr = static_cast< NxU8* >(heightFieldDesc.samples);
		for (uint32_t z = 0; z < heightfield->getSize(); ++z)
		{
			for (uint32_t x = 0; x < heightfield->getSize(); ++x)
			{
				float height = heights[z + x * heightfield->getSize()];

				NxHeightFieldSample* sample = reinterpret_cast< NxHeightFieldSample* >(ptr);

				sample->height = NxI16(height * 32767.5f);
				sample->materialIndex0 = 0;
				sample->materialIndex1 = 0;
				sample->tessFlag = 0;

				ptr += heightFieldDesc.sampleStride;
			}
		}

		NxHeightField* heightFieldData = m_sdk->createHeightField(heightFieldDesc);
		delete[] heightFieldDesc.samples;

		if (!heightFieldData)
		{
			log::error << L"Unable to create heightfield" << Endl;
			return 0;
		}

		NxHeightFieldShapeDesc* heightFieldShapeDesc = new NxHeightFieldShapeDesc();
		heightFieldShapeDesc->heightField = heightFieldData;
		heightFieldShapeDesc->heightScale = heightfield->getWorldExtent().y() / 65536.0f;
		heightFieldShapeDesc->rowScale = heightfield->getWorldExtent().x() / heightfield->getSize();
		heightFieldShapeDesc->columnScale = heightfield->getWorldExtent().z() / heightfield->getSize();
		heightFieldShapeDesc->materialIndexHighBits = 0;
		heightFieldShapeDesc->holeMaterial = 2;
		heightFieldShapeDesc->localPose.t.set(-heightfield->getWorldExtent().x() * 0.5f, 0.0f, -heightfield->getWorldExtent().z() * 0.5f);

		actorShapeDesc = heightFieldShapeDesc;
	}
	else
	{
		log::error << L"Unsupported shape type \"" << type_name(shapeDesc) << L"\"" << Endl;
		return 0;
	}

	if (shapeDesc->getLocalTransform() != Matrix44::identity())
	{
	}

	Ref< Body > body;

	if (const StaticBodyDesc* staticDesc = dynamic_type_cast< const StaticBodyDesc* >(desc))
	{
		NxActorDesc actorDesc;
		actorDesc.shapes.pushBack(actorShapeDesc);
		actorDesc.globalPose.t = actorShapeOffset;

		NxActor* actor = m_scene->createActor(actorDesc);
		if (!actor)
		{
			log::error << L"Unable to create PhysX actor" << Endl;
			return 0;
		}

		setActorGroup(actor, c_defaultGroup);

		Ref< StaticBodyPhysX > staticBody = gc_new< StaticBodyPhysX >(this, actor);
		body = staticBody;

		m_staticBodies.push_back(staticBody);
	}
	else if (const DynamicBodyDesc* dynamicDesc = dynamic_type_cast< const DynamicBodyDesc* >(desc))
	{
		NxBodyDesc bodyDesc;
		bodyDesc.mass = dynamicDesc->getMass();

		NxActorDesc actorDesc;
		actorDesc.shapes.pushBack(actorShapeDesc);
		actorDesc.globalPose.t = actorShapeOffset;
		actorDesc.body = &bodyDesc;
		
		NxActor* actor = m_scene->createActor(actorDesc);
		if (!actor)
		{
			log::error << L"Unable to create PhysX actor" << Endl;
			return 0;
		}

		setActorGroup(actor, c_defaultGroup);

		if (dynamicDesc->getDisabled())
			actor->putToSleep();

		Ref< DynamicBodyPhysX > dynamicBody = gc_new< DynamicBodyPhysX >(this, actor);
		body = dynamicBody;

		m_dynamicBodies.push_back(dynamicBody);
	}
	else
	{
		log::error << L"Unsupported body type \"" << type_name(desc) << L"\"" << Endl;
		return 0;
	}

	delete actorShapeDesc;
	return body;
}

Joint* PhysicsManagerPhysX::createJoint(const JointDesc* desc, const Matrix44& transform, Body* body1, Body* body2)
{
	NxActor* actor1 = 0;
	NxActor* actor2 = 0;

	if (DynamicBodyPhysX* dynamicBody1 = dynamic_type_cast< DynamicBodyPhysX* >(body1))
		actor1 = dynamicBody1->getActor();
	else if (StaticBodyPhysX* staticBody1 = dynamic_type_cast< StaticBodyPhysX* >(body1))
		actor1 = staticBody1->getActor();

	if (DynamicBodyPhysX* dynamicBody2 = dynamic_type_cast< DynamicBodyPhysX* >(body2))
		actor2 = dynamicBody2->getActor();
	else if (StaticBodyPhysX* staticBody2 = dynamic_type_cast< StaticBodyPhysX* >(body2))
		actor2 = staticBody2->getActor();

	Ref< Joint > outJoint;

	if (const BallJointDesc* ballDesc = dynamic_type_cast< const BallJointDesc* >(desc))
	{
		NxSphericalJointDesc jointDesc;
		jointDesc.actor[0] = actor1;
		jointDesc.actor[1] = actor2;
		jointDesc.setGlobalAnchor(toNxVec3(transform * ballDesc->getAnchor().xyz1()));
		
		NxJoint* joint = m_scene->createJoint(jointDesc);
		if (!joint)
			return 0;

		outJoint = gc_new< BallJointPhysX >(this, joint, body1, body2);
	}
	else if (const ConeTwistJointDesc* coneTwistDesc = dynamic_type_cast< const ConeTwistJointDesc* >(desc))
	{
		float coneAngle1, coneAngle2;
		coneTwistDesc->getConeAngles(coneAngle1, coneAngle2);

		NxD6JointDesc jointDesc;
		jointDesc.actor[0] = actor1;
		jointDesc.actor[1] = actor2;
		jointDesc.xMotion = NX_D6JOINT_MOTION_LOCKED;
		jointDesc.yMotion = NX_D6JOINT_MOTION_LOCKED;
		jointDesc.zMotion = NX_D6JOINT_MOTION_LOCKED;
		jointDesc.swing1Motion = NX_D6JOINT_MOTION_LIMITED;
		jointDesc.swing2Motion = NX_D6JOINT_MOTION_LIMITED;
		jointDesc.twistMotion = NX_D6JOINT_MOTION_LIMITED;
		jointDesc.swing1Limit.value = coneAngle1;
		jointDesc.swing2Limit.value = coneAngle2;
		jointDesc.twistLimit.low.value = -coneTwistDesc->getTwistAngle();
		jointDesc.twistLimit.high.value = coneTwistDesc->getTwistAngle();
		jointDesc.setGlobalAnchor(toNxVec3(transform * coneTwistDesc->getAnchor().xyz1()));
		jointDesc.setGlobalAxis(toNxVec3(transform * coneTwistDesc->getTwistAxis().xyz0()));

		NxJoint* joint = m_scene->createJoint(jointDesc);
		if (!joint)
			return 0;

		outJoint = gc_new< ConeTwistJointPhysX >(this, joint, body1, body2);
	}

	return outJoint;
}

void PhysicsManagerPhysX::update()
{
	for (RefArray< DynamicBodyPhysX >::iterator i = m_dynamicBodies.begin(); i != m_dynamicBodies.end(); ++i)
		(*i)->setPreviousState((*i)->getState());

	m_scene->flushStream();
	m_scene->simulate(m_simulationDeltaTime);
	m_scene->fetchResults(NX_RIGID_BODY_FINISHED, true);
}

uint32_t PhysicsManagerPhysX::getCollidingPairs(std::vector< CollisionPair >& outCollidingPairs) const
{
	return 0;
}

bool PhysicsManagerPhysX::queryPoint(const Vector4& at, float margin, QueryResult& outResult) const
{
	return false;
}

bool PhysicsManagerPhysX::queryRay(const Vector4& at, const Vector4& direction, float maxLength, const Body* ignoreBody, QueryResult& outResult) const
{
	NxRaycastHit hit;

	// Move ignore actor into disabled group.
	if (ignoreBody)
	{
		NxActor* actor = static_cast< const DynamicBodyPhysX* >(ignoreBody)->getActor();
		setActorGroup(actor, c_ignoreBodyGroup);
	}

	NxShape* hitShape = m_scene->raycastClosestShape(
		NxRay(toNxVec3(at), toNxVec3(direction)),
		NX_ALL_SHAPES,
		hit,
		(1 << c_defaultGroup),
		maxLength,
		NX_RAYCAST_DISTANCE | NX_RAYCAST_IMPACT | NX_RAYCAST_FACE_NORMAL
	);

	if (hitShape)
	{
		outResult.distance = hit.distance;
		outResult.position = fromNxVec3(hit.worldImpact, 1.0f);
		outResult.normal = fromNxVec3(hit.worldNormal, 0.0f);
	}

	// Restore ignore actor into active group.
	if (ignoreBody)
	{
		NxActor* actor = static_cast< const DynamicBodyPhysX* >(ignoreBody)->getActor();
		T_ASSERT_M (hitShape != *actor->getShapes(), L"Collision with masked shape");

		setActorGroup(actor, c_defaultGroup);
	}

	return bool(hitShape != 0);
}

uint32_t PhysicsManagerPhysX::querySphere(const Vector4& at, float radius, uint32_t queryTypes, RefArray< Body >& outBodies) const
{
	return 0;
}

bool PhysicsManagerPhysX::querySweep(const Vector4& at, const Vector4& direction, float maxLength, float radius, const Body* ignoreBody, QueryResult& outResult) const
{
	return false;
}

void PhysicsManagerPhysX::getBodyCount(uint32_t& outCount, uint32_t& outActiveCount) const
{
}

void PhysicsManagerPhysX::destroyBody(Body* owner, NxActor& actor)
{
	if (StaticBodyPhysX* staticBody = dynamic_type_cast< StaticBodyPhysX* >(owner))
	{
		RefArray< StaticBodyPhysX >::iterator i = std::find(m_staticBodies.begin(), m_staticBodies.end(), staticBody);
		if (i != m_staticBodies.end())
			m_staticBodies.erase(i);
	}
	else if (DynamicBodyPhysX* dynamicBody = dynamic_type_cast< DynamicBodyPhysX* >(owner))
	{
		RefArray< DynamicBodyPhysX >::iterator i = std::find(m_dynamicBodies.begin(), m_dynamicBodies.end(), dynamicBody);
		if (i != m_dynamicBodies.end())
			m_dynamicBodies.erase(i);
	}
	if (m_scene)
		m_scene->releaseActor(actor);
}

void PhysicsManagerPhysX::destroyJoint(Joint* owner, NxJoint& joint)
{
	if (m_scene)
		m_scene->releaseJoint(joint);
}

	}
}
