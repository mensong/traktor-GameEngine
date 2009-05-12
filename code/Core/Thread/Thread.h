#ifndef traktor_Thread_H
#define traktor_Thread_H

#include <string>
#include "Core/Config.h"
#include "Core/Heap/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{

class Functor;

/*! \brief OS thread class.
 * \ingroup Core
 */
class T_DLLCLASS Thread
{
public:
	enum Priority
	{
		Below	= -1,
		Normal	= 0,
		Above	= 1
	};

	bool start(Priority priority = Normal);

	bool wait(int timeout = -1);

	bool stop(int timeout = -1);

	bool pause();

	bool resume();

	void sleep(int duration);

	void yield();

	bool current() const;

	bool stopped() const;

	bool finished() const;

private:
	friend class ThreadManager;

	void* m_handle;
	uint32_t m_id;
	bool m_stopped;
	Functor* m_functor;
	std::string m_name;
	int m_hardwareCore;

	Thread(Functor* functor, const std::wstring& name, int hardwareCore);
	
	~Thread();
};

}

#endif	// traktor_Thread_H
