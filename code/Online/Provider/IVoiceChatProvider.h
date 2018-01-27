/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_online_IVoiceChatProvider_H
#define traktor_online_IVoiceChatProvider_H

#include <vector>
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

class T_DLLCLASS IVoiceChatProvider : public Object
{
	T_RTTI_CLASS;

public:
	struct IVoiceChatCallback
	{
		virtual ~IVoiceChatCallback() {}

		virtual void onVoiceReceived(uint64_t fromUserHandle, const int16_t* samples, uint32_t samplesCount, uint32_t sampleRate) = 0;
	};

	virtual ~IVoiceChatProvider() {}

	virtual void setCallback(IVoiceChatCallback* callback) = 0;

	virtual void setAudience(const std::vector< uint64_t >& audienceHandles) = 0;

	virtual void beginTransmission() = 0;

	virtual void endTransmission() = 0;
};

	}
}

#endif	// traktor_online_IVoiceChatProvider_H
