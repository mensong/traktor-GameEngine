/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
//#include <algorithm>
#include "Core/Containers/SmallMap.h"
//#include "Core/Singleton/ISingleton.h"
//#include "Core/Singleton/SingletonManager.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/CriticalSection.h"
#include "Sound/Types.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

class HandleRegistry
{
public:
	HandleRegistry()
	:	m_nextUnusedHandle(1)
	{
	}

	handle_t getHandle(const std::wstring& name)
	{
		T_ANONYMOUS_VAR(Acquire< CriticalSection >)(m_lock);

		SmallMap< std::wstring, handle_t >::const_iterator i = m_handles.find(name);
		if (i != m_handles.end())
		{
			T_ASSERT (i->second > 0);
			return i->second;
		}

		handle_t handle = m_nextUnusedHandle++;
		m_handles.insert(std::make_pair(name, handle));

		return handle;
	}

private:
	CriticalSection m_lock;
	SmallMap< std::wstring, handle_t > m_handles;
	handle_t m_nextUnusedHandle;
};

HandleRegistry s_handleRegistry;

		}

handle_t getParameterHandle(const std::wstring& name)
{
	return s_handleRegistry.getHandle(name);
}

	}
}
