#include <algorithm>
#include "Database/Local/LocalBus.h"
#include "Core/Serialization/Serializable.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/Serialization/MemberEnum.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/Thread/Acquire.h"
//#include "Core/Io/FileSystem.h"
#include "Core/System/OS.h"
#include "Core/System/SharedMemory.h"
#include "Core/Io/Stream.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace db
	{
		namespace
		{

const Guid c_guidGlobalLock(L"{6DC29473-147F-4b3f-8DF5-BBC7EDF79111}");

class MemberProviderEvent : public MemberEnum< ProviderEvent >
{
public:
	MemberProviderEvent(const std::wstring& name, ProviderEvent& en)
	:	MemberEnum< ProviderEvent >(name, en, ms_keys)
	{
	}

private:
	const static Key ms_keys[];
};

const MemberProviderEvent::Key MemberProviderEvent::ms_keys[] =
{
	{ L"PeInvalid", PeInvalid },
	{ L"PeCommited", PeCommited },
	{ L"PeReverted", PeReverted },
	{ L"PeRenamed", PeRenamed },
	{ L"PeRemoved", PeRemoved },
	0
};

class EventLog : public Serializable
{
	T_RTTI_CLASS(EventLog)

public:
	struct Entry
	{
		Guid sender;
		ProviderEvent event;
		Guid eventId;

		bool serialize(Serializer& s)
		{
			s >> Member< Guid >(L"sender", sender);
			s >> MemberProviderEvent(L"event", event);
			s >> Member< Guid >(L"eventId", eventId);
			return true;
		}
	};

	void registerPeer(const Guid& localGuid)
	{
		m_pending[localGuid].resize(0);
	}

	void unregisterPeer(const Guid& localGuid)
	{
		std::map< Guid, std::list< Entry > >::iterator i = m_pending.find(localGuid);
		if (i != m_pending.end())
			m_pending.erase(i);
	}

	void addEvent(const Guid& senderGuid, ProviderEvent event, const Guid& eventId)
	{
		Entry entry = { senderGuid, event, eventId };
		for (std::map< Guid, std::list< Entry > >::iterator i = m_pending.begin(); i != m_pending.end(); ++i)
			i->second.push_back(entry);
	}

	std::list< Entry >& getEvents(const Guid& localGuid)
	{
		return m_pending[localGuid];
	}

	virtual bool serialize(Serializer& s)
	{
		return s >> MemberStlMap<
			Guid,
			std::list< Entry >,
			MemberStlPair<
				Guid,
				std::list< Entry >,
				Member< Guid >,
				MemberStlList<
					Entry,
					MemberComposite< Entry >
				>
			>
		> (L"pending", m_pending); 
	}

private:
	std::map< Guid, std::list< Entry > > m_pending;
};

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.db.EventLog", EventLog, Serializable)

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.LocalBus", LocalBus, IProviderBus)

LocalBus::LocalBus(const std::wstring& eventFileName)
:	m_localGuid(Guid::create())
,	m_globalLock(c_guidGlobalLock)
{
	Acquire< Mutex > _lock_(m_globalLock);
	Ref< EventLog > eventLog;
	Ref< Stream > eventFile;

	// Create our shared memory object.
	m_shm = OS::getInstance().createSharedMemory(eventFileName, 4UL * 1024 * 1024);
	T_ASSERT (m_shm);

	if (m_globalLock.existing())
	{
		// Read change log.
		eventFile = m_shm->read();
		if (eventFile)
		{
			eventLog = BinarySerializer(eventFile).readObject< EventLog >();
			eventFile->close();
		}
	}

	if (!eventLog)
		eventLog = gc_new< EventLog >();

	// Register ourself with log.
	eventLog->registerPeer(m_localGuid);

	// Write change log.
	eventFile = m_shm->write();
	if (eventFile)
	{
		BinarySerializer(eventFile).writeObject(eventLog);
		eventFile->close();
	}
}

LocalBus::~LocalBus()
{
	m_shm = 0;
}

void LocalBus::close()
{
	Acquire< Mutex > _lock_(m_globalLock);

	// Read change log.
	Ref< EventLog > eventLog;
	Ref< Stream > eventFile = m_shm->read();
	if (eventFile)
	{
		eventLog = BinarySerializer(eventFile).readObject< EventLog >();
		eventFile->close();
	}
	else
		eventLog = gc_new< EventLog >();

	// Remove ourself from log.
	eventLog->unregisterPeer(m_localGuid);

	// Write change log.
	eventFile = m_shm->write();
	if (eventFile)
	{
		BinarySerializer(eventFile).writeObject(eventLog);
		eventFile->close();
	}
}

bool LocalBus::putEvent(ProviderEvent event, const Guid& eventId)
{
	Acquire< Mutex > _lock_(m_globalLock);

	// Read change log.
	Ref< EventLog > eventLog;
	Ref< Stream > eventFile = m_shm->read();
	if (eventFile)
	{
		eventLog = BinarySerializer(eventFile).readObject< EventLog >();
		eventFile->close();
	}
	else
		eventLog = gc_new< EventLog >();

	// Add change to log.
	eventLog->addEvent(m_localGuid, event, eventId);

	// Write change log.
	eventFile = m_shm->write();
	if (!eventFile)
		return false;

	BinarySerializer(eventFile).writeObject(eventLog);
	eventFile->close();

	return true;
}

bool LocalBus::getEvent(ProviderEvent& outEvent, Guid& outEventId, bool& outRemote)
{
	// Pop from local pending events first.
	if (!m_pendingEvents.empty())
	{
		outEvent = m_pendingEvents.front().event;
		outEventId = m_pendingEvents.front().eventId;
		outRemote = m_pendingEvents.front().remote;
		m_pendingEvents.pop_front();
		return true;
	}

	// Read events from global log.
	Acquire< Mutex > _lock_(m_globalLock);

	Ref< Stream > eventFile = m_shm->read();
	if (!eventFile)
		return false;
	
	Ref< EventLog > eventLog = BinarySerializer(eventFile).readObject< EventLog >();

	// Move into local pending events.
	std::list< EventLog::Entry >& events = eventLog->getEvents(m_localGuid);
	for (std::list< EventLog::Entry >::iterator i = events.begin(); i != events.end(); ++i)
	{
		Event event = { i->event, i->eventId, i->sender != m_localGuid };
		m_pendingEvents.push_back(event);
	}
	events.resize(0);
	eventFile->close();

	// Write back event log.
	eventFile = m_shm->write();
	if (eventFile)
	{
		BinarySerializer(eventFile).writeObject(eventLog);
		eventFile->close();
	}

	// Pop from local pending events.
	if (!m_pendingEvents.empty())
	{
		outEvent = m_pendingEvents.front().event;
		outEventId = m_pendingEvents.front().eventId;
		outRemote = m_pendingEvents.front().remote;
		m_pendingEvents.pop_front();
		return true;
	}

	return false;
}

	}
}
