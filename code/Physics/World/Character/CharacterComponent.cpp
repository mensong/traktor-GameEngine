#include "Core/Misc/SafeDestroy.h"
#include "Physics/Body.h"
#include "Physics/PhysicsManager.h"
#include "Physics/World/Character/CharacterComponent.h"
#include "Physics/World/Character/CharacterComponentData.h"
#include "World/Entity.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.CharacterComponent", CharacterComponent, world::IEntityComponent)

CharacterComponent::CharacterComponent(
	PhysicsManager* physicsManager,
	const CharacterComponentData* data,
	Body* bodyWide,
	Body* bodySlim,
	uint32_t traceInclude,
	uint32_t traceIgnore
)
:	m_owner(nullptr)
,	m_physicsManager(physicsManager)
,	m_data(data)
,	m_bodyWide(bodyWide)
,	m_bodySlim(bodySlim)
,	m_traceInclude(traceInclude)
,	m_traceIgnore(traceIgnore)
,	m_headAngle(0.0f)
,	m_velocity(Vector4::zero())
,	m_grounded(false)
{
}

void CharacterComponent::destroy()
{
	safeDestroy(m_bodyWide);
	safeDestroy(m_bodySlim);
	m_owner = nullptr;
}

void CharacterComponent::setOwner(world::Entity* owner)
{
	if ((m_owner = owner) != nullptr)
	{
		Transform transform;
		if (m_owner->getTransform(transform))
		{
			m_bodyWide->setTransform(transform);
			m_bodyWide->setEnable(true);
		}
	}
}

void CharacterComponent::setTransform(const Transform& transform)
{
	m_bodyWide->setTransform(transform);
}

Aabb3 CharacterComponent::getBoundingBox() const
{
	return Aabb3();
}

void CharacterComponent::update(const world::UpdateParams& update)
{
	float dT = update.deltaTime;

	Vector4 movement = m_velocity * Scalar(dT);
	Vector4 position = m_bodyWide->getTransform().translation();
	QueryResult result;

	// Integrate gravity.
	m_velocity += Vector4(0.0f, -9.2f, 0.0f) * Scalar(dT);

	// Step up.
	float stepUpLength = m_data->getStep();
	if (movement.y() > 0.0f)
		stepUpLength += movement.y();

	if (stepVertical(stepUpLength, position))
	{
		// Head hit; cancel out up motion.
		m_velocity *= Vector4(1.0f, 0.0f, 1.0f, 0.0f);
	}

	// Step forward.
	Vector4 movementXZ = movement * Vector4(1.0f, 0.0f, 1.0f);
	step(movementXZ, position);

	// Step down, step further down to simulate falling.
	float stepDownLength = m_data->getStep();
	if (movement.y() < 0.0f)
		stepDownLength += -movement.y();

	if (stepVertical(-stepDownLength, position))
	{
		// Foot hit; cancel out up motion.
		m_velocity *= Vector4(1.0f, 0.0f, 1.0f, 0.0f);
		m_grounded = true;
	}
	else
		m_grounded = false;

	Quaternion rotation = Quaternion::fromEulerAngles(m_headAngle, 0.0f, 0.0f);

	m_bodyWide->setTransform(Transform(
		position,
		rotation
	));
	if (m_owner)
		m_owner->setTransform(Transform(
			position,
			rotation
		));
}

void CharacterComponent::setHeadAngle(float headAngle)
{
	m_headAngle = headAngle;
}

float CharacterComponent::getHeadAngle() const
{
	return m_headAngle;
}

void CharacterComponent::setVelocity(const Vector4& velocity)
{
	m_velocity = velocity;
}

const Vector4& CharacterComponent::getVelocity() const
{
	return m_velocity;
}

bool CharacterComponent::isGrounded() const
{
	return m_grounded;
}

bool CharacterComponent::stepVertical(float motion, Vector4& inoutPosition) const
{
	if (std::abs(motion) <= FUZZY_EPSILON)
		return false;

	float direction = (motion > 0.0f) ? 1.0f : -1.0f;
	bool anyCollision = false;
	QueryResult result;

	if (m_physicsManager->querySweep(
		m_bodySlim,
		Quaternion::identity(),
		inoutPosition,
		Vector4(0.0f, direction, 0.0f),
		std::abs(motion),
		physics::QueryFilter(m_traceInclude, m_traceIgnore),
		result
	))
	{
		inoutPosition += Vector4(0.0f, motion * result.fraction, 0.0f);
		anyCollision = true;
	}
	else
	{
		inoutPosition += Vector4(0.0f, motion, 0.0f);
	}

	return anyCollision;
}

bool CharacterComponent::step(Vector4 motion, Vector4& inoutPosition) const
{
	Scalar totalMotionLength = motion.length();
	if (totalMotionLength <= FUZZY_EPSILON)
		return false;

	motion /= totalMotionLength;

	bool anyCollision = false;
	QueryResult result;

	Scalar motionLength = totalMotionLength;
	for (int32_t i = 0; i < 8 && motionLength > FUZZY_EPSILON; ++i)
	{
		if (m_physicsManager->querySweep(
			m_bodyWide,
			Quaternion::identity(),
			inoutPosition,
			motion,
			motionLength,
			physics::QueryFilter(m_traceInclude, m_traceIgnore),
			result
		))
		{
			Scalar move = Scalar(motionLength * result.fraction);

			// Don't move entire distance to prevent stability issues.
			const Scalar c_fudge(0.01f);
			if (move > c_fudge)
				move -= c_fudge;

			inoutPosition += motion * move;

			// Adjust movement vector.
			Scalar k = dot3(-motion, result.normal);
			motion += result.normal * k;
			if (motion.normalize() <= FUZZY_EPSILON)
				break;

			motionLength -= move;
			anyCollision = true;
		}
		else
		{
			inoutPosition += motion * motionLength;
			motionLength = Scalar(0.0f);
		}
	}

	return anyCollision;
}

	}
}
