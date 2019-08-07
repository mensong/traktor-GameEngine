#pragma once

#include <string>
#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Singleton/ISingleton.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Thread/Signal.h"
#include "Core/Thread/Thread.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_TELEMETRY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace telemetry
	{

class ITask;

/*! \brief
 * \ingroup Telemetry
 */
class T_DLLCLASS Telemetry
:	public Object
,	public ISingleton
{
	T_RTTI_CLASS;

public:
	Telemetry();

	static Telemetry& getInstance();

	bool create(const std::wstring& serverHost, const std::wstring& client, uint32_t sequenceNr);

	void event(const std::wstring& symbol);

	void set(const std::wstring& id, int32_t value);

	void add(const std::wstring& id, int32_t value);

	void flush();

	uint32_t getLastSequenceNumber() const { return m_sequenceNr; }

protected:
	virtual void destroy() override final;

private:
	Semaphore m_lock;
	Thread* m_thread;
	RefArray< ITask > m_queue;
	Signal m_queueSignal;
	int32_t m_queueError;
	std::wstring m_serverHost;
	std::wstring m_client;
	uint32_t m_sequenceNr;

	void threadProcessQueue();
};

	}
}
