/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Ray2.h"

namespace traktor
{

T_MATH_INLINE Ray2::Ray2()
{
}

T_MATH_INLINE Ray2::Ray2(const Vector2& origin_, const Vector2& direction_)
:	origin(origin_)
,	direction(direction_)
{
}

T_MATH_INLINE Vector2 Ray2::normal() const
{
	return direction.perpendicular();
}

T_MATH_INLINE float Ray2::distance(const Vector2& pt) const
{
	return dot(normal(), pt - origin);
}

T_MATH_INLINE bool Ray2::intersect(const Ray2& ray, float& outR, float& outK) const
{
	if (direction.y / direction.x == ray.direction.y / ray.direction.x)
		return false;

	const float num = direction.x * ray.direction.y - direction.y * ray.direction.x;
	if (num == 0.0f)
		return false;

	const Vector2 v = origin - ray.origin;

	outR = (v.y * direction.x - v.x * direction.y) / num;
	outK = (v.y * ray.direction.x - v.x * ray.direction.y) / num;

	return true;
}

T_MATH_INLINE Vector2 Ray2::operator * (float k) const
{
	return origin + direction * k;
}

}
