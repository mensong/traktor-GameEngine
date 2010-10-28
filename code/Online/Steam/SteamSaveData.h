#ifndef traktor_online_SteamSaveData_H
#define traktor_online_SteamSaveData_H

#include "Online/Provider/ISaveDataProvider.h"

namespace traktor
{
	namespace online
	{

class SteamSaveData : public ISaveDataProvider
{
	T_RTTI_CLASS;

public:
	virtual bool enumerate(std::set< std::wstring >& outSaveDataIds);

	virtual bool get(std::wstring& saveDataId, Ref< ISerializable >& outAttachment);

	virtual bool set(std::wstring& saveDataId, const ISerializable* attachment, bool replace);
};

	}
}

#endif	// traktor_online_SteamSaveData_H
