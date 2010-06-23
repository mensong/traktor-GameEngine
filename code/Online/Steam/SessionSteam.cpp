#include <steam/steam_api.h>
#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Online/Steam/CurrentUserSteam.h"
#include "Online/Steam/LeaderboardSteam.h"
#include "Online/Steam/SaveGameSteam.h"
#include "Online/Steam/SessionSteam.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.SessionSteam", SessionSteam, ISession)

SessionSteam::SessionSteam(CurrentUserSteam* user, const std::set< std::wstring >& leaderboards)
:	m_callbackUserStatsReceived(this, &SessionSteam::OnUserStatsReceived)
,	m_callbackUserStatsStored(this, &SessionSteam::OnUserStatsStored)
,	m_callbackAchievementStored(this, &SessionSteam::OnAchievementStored)
,	m_user(user)
,	m_pendingLeaderboards(leaderboards.begin(), leaderboards.end())
,	m_requestedStats(false)
,	m_receivedStats(false)
,	m_storeStats(false)
,	m_storedStats(false)
,	m_requestedLeaderboard(false)
,	m_destroyed(false)
{
}

void SessionSteam::destroy()
{
	if (!m_destroyed)
	{
		m_destroyed = true;

		// Ensure pending stats are sent to Steam.
		if (m_storeStats)
		{
			T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
			if (SteamUser()->BLoggedOn())
			{
				m_storedStats = false;
				if (SteamUserStats()->StoreStats())
				{
					// Wait until we've received an acknowledgment; bail if more than one second.
					for (int i = 0; i < 100; ++i)
					{
						SteamAPI_RunCallbacks();
						if (m_storedStats)
							break;
						ThreadManager::getInstance().getCurrentThread()->sleep(10);
					}
					if (!m_storedStats)
						log::warning << L"Steam stats not stored properly" << Endl;
				}
			}
		}
	}
}

bool SessionSteam::isConnected() const
{
	return SteamUser()->BLoggedOn();
}

Ref< IUser > SessionSteam::getUser()
{
	return m_user;
}

bool SessionSteam::rewardAchievement(const std::wstring& achievementId)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	if (!m_receivedStats)
	{
		log::error << L"Unable to reward achievement \"" << achievementId << L"\"; no stats received yet" << Endl;
		return false;
	}

	if (!SteamUserStats()->SetAchievement(wstombs(achievementId).c_str()))
	{
		log::error << L"Unable to reward achievement \"" << achievementId << L"\"; SetAchievement failed" << Endl;
		return false;
	}

	m_storeStats = true;
	return true;
}

bool SessionSteam::withdrawAchievement(const std::wstring& achievementId)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	if (!m_receivedStats)
	{
		log::error << L"Unable to withdraw achievement \"" << achievementId << L"\"; no stats received yet" << Endl;
		return false;
	}

	if (!SteamUserStats()->ClearAchievement(wstombs(achievementId).c_str()))
	{
		log::error << L"Unable to withdraw achievement \"" << achievementId << L"\"; ClearAchievement failed" << Endl;
		return false;
	}

	m_storeStats = true;
	return true;
}

Ref< ILeaderboard > SessionSteam::getLeaderboard(const std::wstring& id)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	if (!m_pendingLeaderboards.empty())
	{
		log::error << L"Unable to get leaderboard \"" << id << L"\"; pending leaderboards not received yet" << Endl;
		return false;
	}

	std::map< std::wstring, Ref< LeaderboardSteam > >::iterator it = m_leaderboards.find(id);
	return it != m_leaderboards.end() ? it->second : 0;
}

bool SessionSteam::setStatValue(const std::wstring& statId, float value)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	if (!m_receivedStats)
		return false;

	if (!SteamUserStats()->SetStat(wstombs(statId).c_str(), value))
		return false;

	m_storeStats = true;
	return true;
}

bool SessionSteam::getStatValue(const std::wstring& statId, float& outValue)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	if (!m_receivedStats)
		return false;

	return SteamUserStats()->GetStat(wstombs(statId).c_str(), &outValue);
}

Ref< ISaveGame > SessionSteam::createSaveGame(const std::wstring& name, ISerializable* attachment)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	DynamicMemoryStream dms(false, true);
	BinarySerializer(&dms).writeObject(attachment);

	const std::vector< uint8_t >& buffer = dms.getBuffer();
	if (!SteamRemoteStorage()->FileWrite(wstombs(name).c_str(), &buffer[0], buffer.size()))
	{
		log::error << L"Unable to create save game; Steam FileWrite failed" << Endl;
		return 0;
	}

	return new SaveGameSteam(name);
}

bool SessionSteam::getAvailableSaveGames(RefArray< ISaveGame >& outSaveGames) const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	int32_t fileCount = SteamRemoteStorage()->GetFileCount();
	int32_t fileSize;

	for (int32_t i = 0; i < fileCount; ++i)
	{
		const char* fileName = SteamRemoteStorage()->GetFileNameAndSize(i, &fileSize);
		if (!fileName)
			continue;

		outSaveGames.push_back(new SaveGameSteam(mbstows(fileName)));
	}

	return true;
}

bool SessionSteam::update()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	// Request stats; do once at first update.
	if (!m_requestedStats)
	{
		if (SteamUser()->BLoggedOn())
		{
			if (SteamUserStats()->RequestCurrentStats())
				m_requestedStats = true;
		}
	}

	if (m_receivedStats)
	{
		// Store modified stats; only if no pending leaderboard request.
		if (m_storeStats && !m_requestedLeaderboard)
		{
			m_storedStats = false;
			if (SteamUser()->BLoggedOn())
			{
				if (SteamUserStats()->StoreStats())
					m_storeStats = false;
			}
		}
		
		// Request another leaderboard; only when not storing stats.
		if (m_storedStats && !m_requestedLeaderboard)
		{
			if (!m_pendingLeaderboards.empty())
			{
				std::wstring id = m_pendingLeaderboards.back();
				log::info << L"Requesting leaderboard \"" << id << L"\"..." << Endl;

				SteamAPICall_t call = SteamUserStats()->FindOrCreateLeaderboard(
					wstombs(id).c_str(),
					k_ELeaderboardSortMethodDescending,
					k_ELeaderboardDisplayTypeNumeric
				);
				if (call != 0)
				{
					m_pendingLeaderboards.pop_back();
					m_requestedLeaderboard = true;
					m_callbackFindLeaderboard.Set(call, this, &SessionSteam::OnLeaderboardFind);
				}
			}
		}
	}

	return !m_destroyed;
}

void SessionSteam::OnUserStatsReceived(UserStatsReceived_t* pCallback)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	if (pCallback->m_eResult == k_EResultOK)
	{
		log::debug << L"Steam session event; user stats received" << Endl;
		m_receivedStats = true;
	}
	else
		log::error << L"Steam session event; failure receiving stats (m_eResult = " << int32_t(pCallback->m_eResult) << L")" << Endl;
}

void SessionSteam::OnUserStatsStored(UserStatsStored_t* pCallback)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	if (pCallback->m_eResult == k_EResultOK)
	{
		log::debug << L"Steam session event; user stats stored" << Endl;
		m_storedStats = true;
	}
	else
		log::error << L"Steam session event; failure storing stats (m_eResult = " << int32_t(pCallback->m_eResult) << L")" << Endl;
}

void SessionSteam::OnAchievementStored(UserAchievementStored_t* pCallback)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	log::debug << L"Steam session event; achievement stored" << Endl;
	m_storedStats = true;
}

void SessionSteam::OnLeaderboardFind(LeaderboardFindResult_t* pCallback, bool bIOFailure)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	m_callbackFindLeaderboard.Cancel();

	if (pCallback->m_hSteamLeaderboard != 0)
	{
		log::debug << L"Steam session event; found leaderboard, requesting rank/score of current player..." << Endl;

		SteamAPICall_t call = SteamUserStats()->DownloadLeaderboardEntries(pCallback->m_hSteamLeaderboard, k_ELeaderboardDataRequestGlobalAroundUser, 0, 0);
		if (call == 0)
		{
			log::error << L"Steam session event; unable to request rank/score" << Endl;
			m_requestedLeaderboard = false;
			return;
		}

		m_callbackDownloadLeaderboard.Set(call, this, &SessionSteam::OnLeaderboardDownloaded);
	}
	else
	{
		log::error << L"Steam session event; didn't find requested leaderboard" << Endl;
		m_requestedLeaderboard = false;
	}
}

void SessionSteam::OnLeaderboardDownloaded(LeaderboardScoresDownloaded_t* pCallback, bool bIOFailure)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	log::debug << L"Steam session event; got rank/score of current player..." << Endl;

	m_callbackDownloadLeaderboard.Cancel();

	const char* leaderboardId = SteamUserStats()->GetLeaderboardName(pCallback->m_hSteamLeaderboard);
	if (!leaderboardId)
	{
		m_requestedLeaderboard = false;
		return;
	}

	LeaderboardEntry_t leaderboardEntry;
	leaderboardEntry.m_nGlobalRank = 0;
	leaderboardEntry.m_nScore = 0;

	if (pCallback->m_cEntryCount > 0)
		SteamUserStats()->GetDownloadedLeaderboardEntry(pCallback->m_hSteamLeaderboardEntries, 0, &leaderboardEntry, 0, 0);

	Ref< LeaderboardSteam > leaderboard = new LeaderboardSteam(
		pCallback->m_hSteamLeaderboard,
		leaderboardEntry.m_nGlobalRank,
		leaderboardEntry.m_nScore
	);

	m_leaderboards.insert(std::make_pair(
		mbstows(leaderboardId),
		leaderboard
	));

	m_requestedLeaderboard = false;
}

	}
}
