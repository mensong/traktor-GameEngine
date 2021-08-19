#pragma once

#if defined(__PS3__)
#	include <sys/synchronization.h>
#endif
#include "Core/Thread/IWaitable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! Blocking semaphore primitive.
 * \ingroup Core
 */
class T_DLLCLASS Semaphore : public IWaitable
{
public:
	Semaphore();

	virtual ~Semaphore();

	virtual bool wait(int32_t timeout = -1);

	void release();

private:
#if defined(__PS3__)
	sys_lwmutex_t m_mutex;
#else
	void* m_handle;
#endif
};

}

