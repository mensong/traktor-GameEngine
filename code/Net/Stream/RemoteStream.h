#ifndef traktor_net_RemoteStream_H
#define traktor_net_RemoteStream_H

#include "Core/Io/IStream.h"
#include "Net/SocketAddressIPv4.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_NET_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

//class Reader;
//class Writer;

	namespace net
	{

class TcpSocket;

/*! \brief
 * \ingroup Net
 */
class T_DLLCLASS RemoteStream : public IStream
{
	T_RTTI_CLASS;

public:
	static Ref< IStream > connect(const SocketAddressIPv4& addr, uint32_t id);

	virtual ~RemoteStream();

	virtual void close() T_OVERRIDE T_FINAL;

	virtual bool canRead() const T_OVERRIDE T_FINAL;

	virtual bool canWrite() const T_OVERRIDE T_FINAL;

	virtual bool canSeek() const T_OVERRIDE T_FINAL;

	virtual int tell() const T_OVERRIDE T_FINAL;

	virtual int available() const T_OVERRIDE T_FINAL;

	virtual int seek(SeekOriginType origin, int offset) T_OVERRIDE T_FINAL;

	virtual int read(void* block, int nbytes) T_OVERRIDE T_FINAL;

	virtual int write(const void* block, int nbytes) T_OVERRIDE T_FINAL;

	virtual void flush() T_OVERRIDE T_FINAL;

private:
	SocketAddressIPv4 m_addr;
	Ref< TcpSocket > m_socket;
	uint8_t m_status;

	RemoteStream();
};

	}
}

#endif	// traktor_net_RemoteStream_H
