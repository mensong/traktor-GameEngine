#include "Core/Platform.h"
#include "Core/Thread/ThreadLocal.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ThreadLocal", ThreadLocal, Object)

ThreadLocal::ThreadLocal()
{
	pthread_key_t& key = *reinterpret_cast< pthread_key_t* >(&m_handle);
	pthread_key_create(&key, 0);
}

ThreadLocal::~ThreadLocal()
{
	pthread_key_t& key = *reinterpret_cast< pthread_key_t* >(&m_handle);
	pthread_key_delete(key);
}

void ThreadLocal::set(void* ptr)
{
	pthread_key_t& key = *reinterpret_cast< pthread_key_t* >(&m_handle);
	pthread_setspecific(key, ptr);
}

void* ThreadLocal::get() const
{
	const pthread_key_t& key = *reinterpret_cast< const pthread_key_t* >(&m_handle);
	return pthread_getspecific(key);
}

}
