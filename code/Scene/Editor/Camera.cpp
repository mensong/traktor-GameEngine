#include "Scene/Editor/Camera.h"
#include "Core/Math/MathUtils.h"
#include "Core/Math/Const.h"

namespace traktor
{
	namespace scene
	{
		namespace
		{

const float c_cameraSpeed = 3.0f;
const Scalar c_minLookAtDistance(0.1f);

inline void decompose(const Matrix44& transform, Vector4& outPosition, Quaternion& outOrientation)
{
	outPosition = transform.translation();
	outOrientation = Quaternion(transform).inverse();
}

		}

Camera::Camera(const Matrix44& transform)
:	m_lookMode(LmFree)
,	m_enable(false)
{
	decompose(transform.inverse(), m_target.position, m_target.orientation);
	m_current = m_target;
}

void Camera::setEnable(bool enable)
{
	m_enable = enable;
}

bool Camera::isEnable() const
{
	return m_enable;
}

void Camera::enterFreeLook()
{
	m_lookMode = LmFree;
}

void Camera::enterLookAt(const Vector4& lookAtPosition)
{
	m_lookAtPosition = lookAtPosition;
	m_lookAtDistance = (lookAtPosition - m_target.position).length();
	m_lookMode = LmLookAt;
	m_target.orientation = Quaternion(lookAt(m_target.position, m_lookAtPosition));
}

void Camera::move(const Vector4& direction)
{
	switch (m_lookMode)
	{
	case LmFree:
		m_target.position += m_target.orientation.inverse() * direction;
		break;

	case LmLookAt:
		{
			m_lookAtDistance -= direction.z();
			if (m_lookAtDistance < c_minLookAtDistance)
				m_lookAtDistance = c_minLookAtDistance;

			m_target.position += m_target.orientation.inverse() * direction;
			m_target.position = m_lookAtPosition + (m_target.position - m_lookAtPosition).normalized() * m_lookAtDistance;
			m_target.orientation = Quaternion(lookAt(m_target.position, m_lookAtPosition));
		}
		break;
	}
	m_current = m_target;
}

void Camera::rotate(float dy, float dx)
{
	switch (m_lookMode)
	{
	case LmFree:
		m_target.orientation *= Quaternion(Vector4(0.0f, 1.0f, 0.0f, 0.0f), dx);
		m_target.orientation *= Quaternion(m_target.orientation.inverse() * Vector4(1.0f, 0.0f, 0.0f, 0.0f), dy);
		m_target.orientation = m_target.orientation.normalized();
		break;

	case LmLookAt:
		// Do not rotate camera as it's fixed on target position.
		break;
	}
	m_current = m_target;
}

void Camera::setTargetView(const Matrix44& transform)
{
	enterFreeLook();
	decompose(transform.inverse(), m_target.position, m_target.orientation);
}

void Camera::update(float deltaTime)
{
	if (m_current.position != m_target.position || m_current.orientation != m_target.orientation)
	{
		deltaTime = min< float >(deltaTime * c_cameraSpeed, 1.0f);
		m_current.position = lerp(m_current.position, m_target.position, Scalar(deltaTime));
		m_current.orientation = lerp(m_current.orientation, m_target.orientation, deltaTime);
	}
}

Matrix44 Camera::getCurrentWorld() const
{
	return translate(m_current.position) * m_current.orientation.inverse().toMatrix44();
}

Matrix44 Camera::getTargetWorld() const
{
	return translate(m_target.position) * m_target.orientation.inverse().toMatrix44();
}

Matrix44 Camera::getCurrentView() const
{
	return getCurrentWorld().inverse();
}

Matrix44 Camera::getTargetView() const
{
	return getTargetWorld().inverse();
}

	}
}
