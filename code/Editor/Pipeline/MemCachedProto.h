#pragma once

#include <string>
#include "Core/Object.h"
#include "Core/Ref.h"

namespace traktor
{
	namespace net
	{

class Socket;

	}

	namespace editor
	{

class MemCachedProto : public Object
{
	T_RTTI_CLASS;

public:
	explicit MemCachedProto(net::Socket* socket);

	bool sendCommand(const std::string& command);

	bool readReply(std::string& outReply);

	bool readData(uint8_t* data, uint32_t dataLength);

	/*! Write data to memcached.
	 * \note Data block must contain space for two more
	 *       bytes as we need to append \r\n.
	 */
	bool writeData(uint8_t* data, uint32_t dataLength);

private:
	Ref< net::Socket > m_socket;
};

	}
}

