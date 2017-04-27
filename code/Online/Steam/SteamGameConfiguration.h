/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_online_SteamGameConfiguration_H
#define traktor_online_SteamGameConfiguration_H

#include <list>
#include <map>
#include "Online/IGameConfiguration.h"

namespace traktor
{
	namespace online
	{

class SteamGameConfiguration : public IGameConfiguration
{
	T_RTTI_CLASS;

public:
	SteamGameConfiguration();

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	friend class SteamSessionManager;

	uint32_t m_appId;
	uint32_t m_requestAttempts;
	bool m_drmEnabled;
	bool m_cloudEnabled;
	bool m_allowP2PRelay;
	std::list< std::wstring > m_achievementIds;
	std::list< std::wstring > m_leaderboardIds;
	std::list< std::wstring > m_statsIds;
	std::map< std::wstring, uint32_t > m_dlcIds;
};

	}
}

#endif	// traktor_online_SteamGameConfiguration_H
