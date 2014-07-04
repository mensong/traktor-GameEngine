#include "Core/Log/Log.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberEnum.h"
#include "Core/Serialization/MemberStl.h"
#include "Flash/Path.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

class MemberVector2i : public MemberComplex
{
public:
	MemberVector2i(const wchar_t* const name, Vector2i& ref)
	:	MemberComplex(name, true)
	,	m_ref(ref)
	{
	}

	virtual void serialize(ISerializer& s) const
	{
		s >> Member< int32_t >(L"x", m_ref.x);
		s >> Member< int32_t >(L"y", m_ref.y);
	}

private:
	Vector2i& m_ref;
};

		}

void SubPathSegment::serialize(ISerializer& s)
{
	s >> MemberEnumByValue< SubPathSegmentType, uint8_t >(L"type", type);
	s >> Member< uint32_t >(L"pointsOffset", pointsOffset);
	s >> Member< uint32_t >(L"pointsCount", pointsCount);
}

void SubPath::serialize(ISerializer& s)
{
	s >> Member< uint16_t >(L"fillStyle0", fillStyle0);
	s >> Member< uint16_t >(L"fillStyle1", fillStyle1);
	s >> Member< uint16_t >(L"lineStyle", lineStyle);
	s >> MemberStlVector< SubPathSegment, MemberComposite< SubPathSegment > >(L"segments", segments);
}

Path::Path()
:	m_cursor(0, 0)
{
	m_points.reserve(256);
}

void Path::reset()
{
	m_points.resize(0);
	m_subPaths.resize(0);
	m_current.segments.resize(0);
}

void Path::moveTo(int32_t x, int32_t y, CoordinateMode mode)
{
	T_ASSERT (m_current.segments.empty());
	transform(mode, CmAbsolute, x, y);
	m_cursor = Vector2i(x, y);
}

void Path::lineTo(int32_t x, int32_t y, CoordinateMode mode)
{
	transform(mode, CmAbsolute, x, y);

	Vector2i p(x, y);
	if (p != m_cursor)
	{
		uint32_t offset = uint32_t(m_points.size());
		m_points.push_back(m_cursor);
		m_points.push_back(p);

		m_current.segments.push_back(SubPathSegment(SpgtLinear));
		m_current.segments.back().pointsOffset = offset;
		m_current.segments.back().pointsCount = 2;

		m_cursor = p;
	}
}

void Path::quadraticTo(int32_t x1, int32_t y1, int32_t x, int32_t y, CoordinateMode mode)
{
	transform(mode, CmAbsolute, x1, y1);
	transform(mode, CmAbsolute, x, y);

	Vector2i p(x, y);
	if (p != m_cursor)
	{
		uint32_t offset = uint32_t(m_points.size());
		m_points.push_back(m_cursor);
		m_points.push_back(Vector2i(x1, y1));
		m_points.push_back(p);

		m_current.segments.push_back(SubPathSegment(SpgtQuadratic));
		m_current.segments.back().pointsOffset = offset;
		m_current.segments.back().pointsCount = 3;

		m_cursor = p;
	}
}

void Path::end(uint16_t fillStyle0, uint16_t fillStyle1, uint16_t lineStyle)
{
	m_current.fillStyle0 = fillStyle0;
	m_current.fillStyle1 = fillStyle1;
	m_current.lineStyle = lineStyle;

	if (!m_current.segments.empty())
	{
		m_subPaths.push_back(m_current);
		m_current.segments.resize(0);
	}
}

Aabb2 Path::getBounds() const
{
	Aabb2 bounds;
	for (std::vector< Vector2i >::const_iterator i = m_points.begin(); i != m_points.end(); ++i)
		bounds.contain(i->toVector2());
	return bounds;
}

void Path::serialize(ISerializer& s)
{
	s >> MemberVector2i(L"cursor", m_cursor);
	s >> MemberStlVector< Vector2i, MemberVector2i >(L"points", m_points);
	s >> MemberStlList< SubPath, MemberComposite< SubPath > >(L"subPaths", m_subPaths);
	s >> MemberComposite< SubPath >(L"current", m_current);
}

void Path::transform(CoordinateMode from, CoordinateMode to, int32_t& x, int32_t& y) const
{
	if (from == to)
		return;

	switch (from)
	{
	case CmRelative:
		x += m_cursor.x;
		y += m_cursor.y;
		break;

	case CmAbsolute:
		x -= m_cursor.x;
		y -= m_cursor.y;
		break;
	}
}

	}
}
