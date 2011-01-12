#ifndef traktor_amalgam_TargetManagerConnection_H
#define traktor_amalgam_TargetManagerConnection_H

#include "Net/SocketStream.h"
#include "Net/TcpSocket.h"
#include "Amalgam/Impl/TargetPerformance.h"

namespace traktor
{
	namespace amalgam
	{

class TargetManagerConnection : public Object
{
	T_RTTI_CLASS;

public:
	TargetManagerConnection();

	bool connect(const std::wstring& host, uint16_t port, const Guid& id);

	void setPerformance(const TargetPerformance& performance);

	void update();

private:
	Ref< net::TcpSocket > m_socket;
	Ref< net::SocketStream > m_socketStream;
	TargetPerformance m_performance;
};

	}
}

#endif	// traktor_amalgam_TargetManagerConnection_H
