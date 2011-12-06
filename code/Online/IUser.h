#ifndef traktor_online_IUser_H
#define traktor_online_IUser_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace online
	{

class T_DLLCLASS IUser : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool getName(std::wstring& outName) const = 0;

	virtual bool sendP2PData(const void* data, size_t size, bool reliable) = 0;
};

	}
}

#endif	// traktor_online_IUser_H
