#include "Core/Object.h"
#include "Core/Memory/Alloc.h"
#include "Core/Memory/FastAllocator.h"
#include "Core/Memory/StdAllocator.h"
#include "Core/Memory/TrackAllocator.h"

namespace traktor
{
	namespace
	{

#pragma pack(1)

const uint32_t c_magic = 'TRKT';

/*! \brief Prepended on all heap allocated objects.
 *
 * \note
 * Must be a multiple of maximum alignment requirement.
 */
struct ObjectHeader
{
	uint32_t magic;
	uint8_t reserved[12];
};

#pragma pack()

inline bool isObjectHeapAllocated(const void* ptr)
{
	const ObjectHeader* header = reinterpret_cast< const ObjectHeader* >(ptr) - 1;
	return bool(header->magic == c_magic);
}

IAllocator* getAllocator()
{
	static IAllocator* s_allocator = 0;
	if (!s_allocator)
	{
#if !defined(_DEBUG)
		s_allocator = new FastAllocator(
			new StdAllocator()
		);
#else
		s_allocator = new TrackAllocator(
			new StdAllocator()
		);
#endif
	}
	return s_allocator;
}

	}

T_IMPLEMENT_RTTI_CLASS_ROOT(L"traktor.Object", Object)

void Object::addRef() const
{
	++m_refCount;
}

void Object::release() const
{
	if (--m_refCount == 0)
	{
		if (isObjectHeapAllocated(this))
			delete this;
	}
}

void* Object::operator new (size_t size)
{
	const size_t objectHeaderSize = sizeof(ObjectHeader);
	
	IAllocator* allocator = getAllocator();

	ObjectHeader* header = static_cast< ObjectHeader* >(allocator->alloc(size + objectHeaderSize, 16, L"Object"));
	T_FATAL_ASSERT_M (header, L"Out of memory");

	Object* object = reinterpret_cast< Object* >(header + 1);

	header->magic = c_magic;
	++ms_instanceCount;

	return object;
}

void Object::operator delete (void* ptr)
{
	if (ptr)
	{
		ObjectHeader* header = static_cast< ObjectHeader* >(ptr) - 1;
		T_ASSERT (header->magic == c_magic);

		--ms_instanceCount;

		IAllocator* allocator = getAllocator();
		allocator->free(header);
	}
}

int32_t Object::getInstanceCount()
{
	return ms_instanceCount;
}

AtomicRefCount Object::ms_instanceCount;

}
