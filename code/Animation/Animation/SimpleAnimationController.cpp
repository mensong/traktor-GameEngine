#include "Animation/SkeletonUtils.h"
#include "Animation/Animation/Animation.h"
#include "Animation/Animation/SimpleAnimationController.h"
#include "Core/Math/Random.h"

namespace traktor
{
	namespace animation
	{
		namespace
		{

Random s_random;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.SimpleAnimationController", SimpleAnimationController, IPoseController)

SimpleAnimationController::SimpleAnimationController(const resource::Proxy< Animation >& animation, bool linearInterpolation)
:	m_animation(animation)
,	m_linearInterpolation(linearInterpolation)
,	m_indexHint(-1)
,	m_lastTime(std::numeric_limits< float >::max())
{
	m_timeOffset = s_random.nextFloat() * m_animation->getLastKeyPose().at;
}

void SimpleAnimationController::destroy()
{
}

void SimpleAnimationController::setTransform(const Transform& transform)
{
}

bool SimpleAnimationController::evaluate(
	float time,
	float deltaTime,
	const Transform& worldTransform,
	const Skeleton* skeleton,
	const AlignedVector< Transform >& jointTransforms,
	AlignedVector< Transform >& outPoseTransforms
)
{
	if (!m_animation)
		return false;

	float poseTime = std::fmod(m_timeOffset + time, m_animation->getLastKeyPose().at);

	Pose pose;
	m_animation->getPose(poseTime, m_linearInterpolation, m_indexHint, pose);

	calculatePoseTransforms(
		skeleton,
		&pose,
		outPoseTransforms
	);

	const bool wrapped = (bool)(poseTime < m_lastTime + FUZZY_EPSILON);
	m_lastTime = poseTime;

	// Assume animation is not continous when wrapping.
	return !wrapped;
}

void SimpleAnimationController::estimateVelocities(
	const Skeleton* skeleton,
	AlignedVector< Velocity >& outVelocities
)
{
}

	}
}
