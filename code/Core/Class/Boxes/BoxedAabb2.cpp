#include "Core/Class/BoxedAllocator.h"
#include "Core/Class/Boxes/BoxedAabb2.h"

namespace traktor
{
	namespace
	{
	
BoxedAllocator< BoxedAabb2, 64 > s_allocBoxedAabb2;
	
	}
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.Aabb2", BoxedAabb2, Boxed)

BoxedAabb2::BoxedAabb2()
{
}

BoxedAabb2::BoxedAabb2(const Aabb2& value)
:	m_value(value)
{
}

BoxedAabb2::BoxedAabb2(const BoxedVector2* min, const BoxedVector2* max)
:	m_value(min->unbox(), max->unbox())
{
}

std::wstring BoxedAabb2::toString() const
{
	return L"(aabb2)";
}

void* BoxedAabb2::operator new (size_t size)
{
	return s_allocBoxedAabb2.alloc();
}

void BoxedAabb2::operator delete (void* ptr)
{
	s_allocBoxedAabb2.free(ptr);
}

}
