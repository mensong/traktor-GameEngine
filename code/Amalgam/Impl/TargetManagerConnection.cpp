#include "Amalgam/Impl/TargetID.h"
#include "Amalgam/Impl/TargetLog.h"
#include "Amalgam/Impl/TargetManagerConnection.h"
#include "Core/Log/Log.h"
#include "Net/BidirectionalObjectTransport.h"
#include "Net/Network.h"
#include "Net/SocketAddressIPv4.h"
#include "Net/TcpSocket.h"

namespace traktor
{
	namespace amalgam
	{
		namespace
		{

class LogRedirect : public ILogTarget
{
public:
	LogRedirect(
		ILogTarget* originalTarget,
		net::BidirectionalObjectTransport* transport
	)
	:	m_originalTarget(originalTarget)
	,	m_transport(transport)
	{
	}

	virtual void log(int32_t level, const std::wstring& str)
	{
		if (m_originalTarget)
			m_originalTarget->log(level, str);

		if (m_transport->connected())
		{
			const TargetLog tlog(level, str);
			m_transport->send(&tlog);
		}
	}

private:
	Ref< ILogTarget > m_originalTarget;
	Ref< net::BidirectionalObjectTransport > m_transport;
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.TargetManagerConnection", TargetManagerConnection, Object)

bool TargetManagerConnection::connect(const std::wstring& host, uint16_t port, const Guid& id)
{
	if (!net::Network::initialize())
	{
		log::error << L"Failed to create target manager connection; network initialization failed" << Endl;
		return false;
	}

	Ref< net::TcpSocket > socket = new net::TcpSocket();
	if (!socket->connect(net::SocketAddressIPv4(host, port)))
	{
		log::error << L"Failed to create target manager connection; unable to connect" << Endl;
		return false;
	}

	socket->setNoDelay(true);

	m_transport = new net::BidirectionalObjectTransport(socket);

    TargetID targetId(id);
	if (!m_transport->send(&targetId))
	{
		log::error << L"Failed to create target manager connection; unable to send ID" << Endl;
		m_transport = 0;
		return false;
	}

	log::info   .setGlobalTarget(new LogRedirect(log::info   .getGlobalTarget(), m_transport));
	log::warning.setGlobalTarget(new LogRedirect(log::warning.getGlobalTarget(), m_transport));
	log::error  .setGlobalTarget(new LogRedirect(log::error  .getGlobalTarget(), m_transport));

	log::info << L"Connection to target manager established successfully" << Endl;
	return true;
}

bool TargetManagerConnection::connected() const
{
	return m_transport != 0;
}

bool TargetManagerConnection::update()
{
	if (!m_transport)
		return false;

	if (!m_transport->connected())
	{
		m_transport = 0;
		return false;
	}

	return true;
}

	}
}
