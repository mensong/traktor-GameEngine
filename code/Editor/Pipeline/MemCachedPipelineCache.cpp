#include <sstream>
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/TString.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Thread/Acquire.h"
#include "Editor/Pipeline/MemCachedPipelineCache.h"
#include "Editor/Pipeline/MemCachedProto.h"
#include "Editor/Pipeline/MemCachedGetStream.h"
#include "Editor/Pipeline/MemCachedPutStream.h"
#include "Net/SocketAddressIPv4.h"
#include "Net/TcpSocket.h"

namespace traktor
{
	namespace editor
	{
		namespace
		{

std::string generateKey(const Guid& guid, const PipelineDependencyHash& hash)
{
	std::stringstream ss;
	ss << wstombs(guid.format()) << ":" << hash.pipelineHash << ":" << hash.sourceAssetHash << ":" << hash.sourceDataHash << ":" << hash.filesHash;
	return ss.str();
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.MemCachedPipelineCache", MemCachedPipelineCache, IPipelineCache)

MemCachedPipelineCache::~MemCachedPipelineCache()
{
	destroy();
}

bool MemCachedPipelineCache::create(const PropertyGroup* settings)
{
	std::wstring host = settings->getProperty< std::wstring >(L"Pipeline.MemCached.Host");
	int32_t port = settings->getProperty< int32_t >(L"Pipeline.MemCached.Port", 11211);
	m_addr = net::SocketAddressIPv4(host, port);
	m_accessRead = settings->getProperty< bool >(L"Pipeline.MemCached.Read", true);
	m_accessWrite = settings->getProperty< bool >(L"Pipeline.MemCached.Write", true);
	return true;
}

void MemCachedPipelineCache::destroy()
{
	m_protos.clear();
}

Ref< IStream > MemCachedPipelineCache::get(const Guid& guid, const PipelineDependencyHash& hash)
{
	if (m_accessRead)
	{
		Ref< MemCachedProto > proto = acquireProto();
		if (!proto)
			return nullptr;

		Ref< MemCachedGetStream > stream = new MemCachedGetStream(this, proto, generateKey(guid, hash));

		// Request end block; do not try to open non-finished, uncommitted cache streams.
		if (!stream->requestEndBlock())
			return nullptr;

		// Request first block of data.
		if (!stream->requestNextBlock())
			return nullptr;

		return stream;
	}
	else
		return nullptr;
}

Ref< IStream > MemCachedPipelineCache::put(const Guid& guid, const PipelineDependencyHash& hash)
{
	if (m_accessWrite)
	{
		Ref< MemCachedProto > proto = acquireProto();
		if (proto)
			return new MemCachedPutStream(this, proto, generateKey(guid, hash));
	}
	return nullptr;
}

bool MemCachedPipelineCache::commit(const Guid& guid, const PipelineDependencyHash& hash)
{
	std::stringstream ss;
	std::string command;
	std::string reply;

	Ref< MemCachedProto > proto = acquireProto();
	if (!proto)
		return false;

	ss << "set " << generateKey(guid, hash) << ":END 0 0 1";

	command = ss.str();
	T_DEBUG(mbstows(command));

	if (!proto->sendCommand(command))
	{
		log::error << L"Unable to store cache block; unable to send command." << Endl;
		return false;
	}

	uint8_t endData[3] = { 0x22, 0x00, 0x00 };
	if (!proto->writeData(endData, 1))
	{
		log::error << L"Unable to store cache block; unable to write data." << Endl;
		return false;
	}

	if (!proto->readReply(reply))
	{
		log::error << L"Unable to store cache block; unable to read reply." << Endl;
		return false;
	}

	if (reply != "STORED")
	{
		log::error << L"Unable to store cache block; server unable to store data." << Endl;
		return false;
	}

	return true;
}

Ref< MemCachedProto > MemCachedPipelineCache::acquireProto()
{
	Ref< MemCachedProto > proto;

	// Try to get an already established connection.
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		if (!m_protos.empty())
		{
			proto = m_protos.back();
			m_protos.pop_back();
		}
	}

	// If no existing connection found then create a new one.
	if (!proto)
	{
		Ref< net::TcpSocket > socket = new net::TcpSocket();
		if (!socket->connect(m_addr))
			return nullptr;
		proto = new MemCachedProto(socket);
	}

	return proto;
}

	}
}
