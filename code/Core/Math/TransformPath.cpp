#include <limits>
#include "Core/Math/TcbSpline.h"
#include "Core/Math/TransformPath.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"

namespace traktor
{
	namespace
	{

Quaternion orientationAsQuaternion(const Vector4& orientation)
{
	return Quaternion(orientation.x(), orientation.y(), orientation.z());
}

struct FrameAccessor
{
	static inline Scalar time(const TransformPath::Key& key) {
		return Scalar(key.T);
	}

	static inline Scalar tension(const TransformPath::Key& key) {
		return key.tcb.x();
	}

	static inline Scalar continuity(const TransformPath::Key& key) {
		return key.tcb.y();
	}

	static inline Scalar bias(const TransformPath::Key& key) {
		return key.tcb.z();
	}

	static inline const TransformPath::Frame& value(const TransformPath::Key& key) {
		return key.value;
	}

	static inline TransformPath::Frame combine(
		const TransformPath::Frame& v0, const Scalar& w0,
		const TransformPath::Frame& v1, const Scalar& w1,
		const TransformPath::Frame& v2, const Scalar& w2,
		const TransformPath::Frame& v3, const Scalar& w3
	)
	{
		TransformPath::Frame f;
		f.position = v0.position * w0 + v1.position * w1 + v2.position * w2 + v3.position * w3;
		f.orientation = v0.orientation * w0 + v1.orientation * w1 + v2.orientation * w2 + v3.orientation * w3;
		return f;
	}
};

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.TransformPath", 0, TransformPath, ISerializable)

void TransformPath::insert(float at, const Frame& frame)
{
	Key key;
	key.T = at;
	key.value = frame;

	size_t keys = m_keys.size();
	if (keys >= 1)
	{
		if (at <= m_keys.front().T)
			m_keys.insert(m_keys.begin(), key);
		else if (at >= m_keys.back().T)
			m_keys.insert(m_keys.end(), key);
		else
		{
			for (size_t i = 0; i < keys - 1; ++i)
			{
				const Key& k1 = m_keys[i];
				const Key& k2 = m_keys[i + 1];
				if (at > k1.T && at < k2.T)
				{
					AlignedVector< Key >::iterator iter = m_keys.begin(); std::advance(iter, i + 1);
					m_keys.insert(iter, key);
					break;
				}
			}
		}
	}
	else
	{
		m_keys.push_back(key);
	}
}

TransformPath::Frame TransformPath::evaluate(float at, bool loop) const
{
	float Tend = getEndTime() + getStartTime();
	return evaluate(at, Tend, loop);
}

TransformPath::Frame TransformPath::evaluate(float at, float end, bool loop) const
{
	size_t nkeys = m_keys.size();
	if (nkeys == 0)
		return Frame();
	else if (nkeys == 1)
		return m_keys[0].value;
	else
	{
		Frame frame = loop ?
			TcbSpline< Key, Scalar, Scalar, Frame, FrameAccessor, WrapTime< Scalar > >::evaluate(&m_keys[0], m_keys.size(), Scalar(at), Scalar(end)) :
			TcbSpline< Key, Scalar, Scalar, Frame, FrameAccessor, ClampTime< Scalar > >::evaluate(&m_keys[0], m_keys.size(), Scalar(at), Scalar(end));

		frame.position = frame.position.xyz1();
		frame.orientation = frame.orientation.xyz0();

		return frame;
	}
}

TransformPath::Key* TransformPath::getClosestKey(float at)
{
	if (m_keys.empty())
		return 0;

	float minT = std::numeric_limits< float >::max();
	uint32_t minI = 0;

	for (uint32_t i = 0; i < uint32_t(m_keys.size()); ++i)
	{
		float dT = abs(at - m_keys[i].T);
		if (dT < minT)
		{
			minT = dT;
			minI = i;
		}
	}

	return &m_keys[minI];
}

TransformPath::Key* TransformPath::getClosestPreviousKey(float at)
{
	if (m_keys.empty())
		return 0;

	float minT = std::numeric_limits< float >::max();
	Key* minK = 0;

	for (uint32_t i = 0; i < uint32_t(m_keys.size()); ++i)
	{
		if (at <= m_keys[i].T + FUZZY_EPSILON)
			continue;

		float dT = at - m_keys[i].T;
		if (dT < minT)
		{
			minT = dT;
			minK = &m_keys[i];
		}
	}

	return minK;
}

TransformPath::Key* TransformPath::getClosestNextKey(float at)
{
	if (m_keys.empty())
		return 0;

	float minT = std::numeric_limits< float >::max();
	Key* minK = 0;

	for (uint32_t i = 0; i < uint32_t(m_keys.size()); ++i)
	{
		if (at >= m_keys[i].T - FUZZY_EPSILON)
			continue;

		float dT = m_keys[i].T - at;
		if (dT < minT)
		{
			minT = dT;
			minK = &m_keys[i];
		}
	}

	return minK;
}

TransformPath::Frame* TransformPath::getClosestKeyFrame(float at)
{
	TransformPath::Key* closestKey = getClosestKey(at);
	return closestKey ? &closestKey->value : 0;
}

bool TransformPath::serialize(ISerializer& s)
{
	return s >> MemberAlignedVector< Key, MemberComposite< Key > >(L"keys", m_keys);
}

bool TransformPath::Key::serialize(ISerializer& s)
{
	s >> Member< float >(L"T", T, 0.0f);
	s >> Member< Vector4 >(L"tcb", tcb);
	s >> MemberComposite< Frame >(L"value", value);
	return true;
}

Transform TransformPath::Frame::transform() const
{
	return Transform(
		position.xyz0(),
		Quaternion(
			orientation.x(),
			orientation.y(),
			orientation.z()
		)
	);
}

bool TransformPath::Frame::serialize(ISerializer& s)
{
	s >> Member< Vector4 >(L"position", position);
	s >> Member< Vector4 >(L"orientation", orientation);
	return true;
}

}
