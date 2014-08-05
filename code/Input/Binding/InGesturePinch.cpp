#include "Core/Containers/CircularVector.h"
#include "Core/Log/Log.h"
#include "Core/Math/Vector2.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberEnum.h"
#include "Core/Serialization/MemberRef.h"
#include "Input/Binding/InGesturePinch.h"
#include "Input/Binding/InputValueSet.h"
#include "Input/Binding/ValueDigital.h"

namespace traktor
{
	namespace input
	{
		namespace
		{

const float c_sampleThreshold = 2.0f;
const float c_deviateThreshold = 100.0f;

struct InGesturePinchInstance : public RefCountImpl< IInputNode::Instance >
{
	Ref< IInputNode::Instance > sourceActiveInstance;
	Ref< IInputNode::Instance > sourceX1Instance;
	Ref< IInputNode::Instance > sourceY1Instance;
	Ref< IInputNode::Instance > sourceX2Instance;
	Ref< IInputNode::Instance > sourceY2Instance;
	CircularVector< Vector2, 128 > points1;
	CircularVector< Vector2, 128 > points2;
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.InGesturePinch", 0, InGesturePinch, IInputNode)

InGesturePinch::InGesturePinch()
{
}

InGesturePinch::InGesturePinch(
	IInputNode* sourceActive,
	IInputNode* sourceX1,
	IInputNode* sourceY1,
	IInputNode* sourceX2,
	IInputNode* sourceY2
)
:	m_sourceActive(sourceActive)
,	m_sourceX1(sourceX1)
,	m_sourceY1(sourceY1)
,	m_sourceX2(sourceX2)
,	m_sourceY2(sourceY2)
{
}

Ref< IInputNode::Instance > InGesturePinch::createInstance() const
{
	Ref< InGesturePinchInstance > instance = new InGesturePinchInstance();
	instance->sourceActiveInstance = m_sourceActive->createInstance();
	instance->sourceX1Instance = m_sourceX1->createInstance();
	instance->sourceY1Instance = m_sourceY1->createInstance();
	instance->sourceX2Instance = m_sourceX2->createInstance();
	instance->sourceY2Instance = m_sourceY2->createInstance();
	return instance;
}

float InGesturePinch::evaluate(
	Instance* instance,
	const InputValueSet& valueSet,
	float T,
	float dT
) const
{
	InGesturePinchInstance* pinchInstance = static_cast< InGesturePinchInstance* >(instance);

	float V = m_sourceActive->evaluate(pinchInstance->sourceActiveInstance, valueSet, T, dT);
	if (!asBoolean(V))
	{
		float sign = 0.0f;

		// If any points has been sampled we need to match with pinch gesture.
		if (!pinchInstance->points1.empty() && !pinchInstance->points2.empty())
		{
			const CircularVector< Vector2, 128 >& points1 = pinchInstance->points1;
			const CircularVector< Vector2, 128 >& points2 = pinchInstance->points2;

			// Determine if sampled points match desired pinch gesture.
			if (points1.size() >= 3 && points2.size() >= 3)
			{
				// Determine pinch direction from first and last point.
				Vector2 dir1 = (points1.back() - points1.front()).normalized();
				Vector2 dir2 = (points2.back() - points2.front()).normalized();

				bool result = bool(dot(dir1, dir2) < 0.0f);
				if (result)
				{
					// Calculate point set average mid point.
					Vector2 mid1(0.0f, 0.0f);

					for (uint32_t i = 0; i < points1.size(); ++i)
						mid1 += points1[i];

					mid1 /= float(points1.size());

					Vector2 mid2(0.0f, 0.0f);

					for (uint32_t i = 0; i < points2.size(); ++i)
						mid2 += points2[i];

					mid2 /= float(points2.size());

					// Ensure no point deviate too much from direction.
					for (uint32_t i = 0; i < points1.size(); ++i)
					{
						float k = abs(dot(dir1.perpendicular(), points1[i] - mid1));
						result &= bool(k <= c_deviateThreshold);
					}
					for (uint32_t i = 0; i < points2.size(); ++i)
					{
						float k = abs(dot(dir2.perpendicular(), points2[i] - mid2));
						result &= bool(k <= c_deviateThreshold);
					}

					if (result)
					{
						Vector2 mid = (mid1 + mid2) / 2.0f;

						float k1 = dot(mid - mid1, dir1);
						float k2 = dot(mid - mid2, dir2);

						if (k1 > 0.0f && k2 > 0.0f)
							sign = -1.0f;
						else if (k1 < 0.0f && k2 < 0.0f)
							sign = 1.0f;
					}
				}
			}

			pinchInstance->points1.clear();
			pinchInstance->points2.clear();
		}

		return sign;
	}

	// Pinch is active; sample positions.
	float X1 = m_sourceX1->evaluate(pinchInstance->sourceX1Instance, valueSet, T, dT);
	float Y1 = m_sourceY1->evaluate(pinchInstance->sourceY1Instance, valueSet, T, dT);

	if (!pinchInstance->points1.empty())
	{
		// Ensure distance from last sample.
		Vector2 delta = Vector2(X1, Y1) - pinchInstance->points1.back();
		if (delta.length() >= c_sampleThreshold)
			pinchInstance->points1.push_back(Vector2(X1, Y1));
	}
	else
		pinchInstance->points1.push_back(Vector2(X1, Y1));

	float X2 = m_sourceX2->evaluate(pinchInstance->sourceX2Instance, valueSet, T, dT);
	float Y2 = m_sourceY2->evaluate(pinchInstance->sourceY2Instance, valueSet, T, dT);

	if (!pinchInstance->points2.empty())
	{
		// Ensure distance from last sample.
		Vector2 delta = Vector2(X2, Y2) - pinchInstance->points2.back();
		if (delta.length() >= c_sampleThreshold)
			pinchInstance->points2.push_back(Vector2(X2, Y2));
	}
	else
		pinchInstance->points2.push_back(Vector2(X2, Y2));

	return asFloat(false);
}

void InGesturePinch::serialize(ISerializer& s)
{
	s >> MemberRef< IInputNode >(L"sourceActive", m_sourceActive);
	s >> MemberRef< IInputNode >(L"sourceX1", m_sourceX1);
	s >> MemberRef< IInputNode >(L"sourceY1", m_sourceY1);
	s >> MemberRef< IInputNode >(L"sourceX2", m_sourceX2);
	s >> MemberRef< IInputNode >(L"sourceY2", m_sourceY2);
}
	
	}
}
