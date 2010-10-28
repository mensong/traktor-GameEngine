#ifndef traktor_online_SteamAchievements_H
#define traktor_online_SteamAchievements_H

#include <steam/steam_api.h>
#include "Online/Provider/IAchievementsProvider.h"

namespace traktor
{
	namespace online
	{

class SteamSessionManager;

class SteamAchievements : public IAchievementsProvider
{
	T_RTTI_CLASS;

public:
	SteamAchievements(SteamSessionManager* sessionManager, const wchar_t** achievementIds);

	virtual bool enumerate(std::map< std::wstring, bool >& outAchievements);

	virtual bool set(const std::wstring& achievementId, bool reward);

private:
	SteamSessionManager* m_sessionManager;
	std::set< std::wstring > m_achievementIds;
	bool m_storeAchievement;

	STEAM_CALLBACK(SteamAchievements, OnAchievementStored, UserAchievementStored_t, m_callbackAchievementStored);
};

	}
}

#endif	// traktor_online_SteamAchievements_H
