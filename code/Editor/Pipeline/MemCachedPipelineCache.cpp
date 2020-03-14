#include <sstream>
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/TString.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
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

MemCachedPipelineCache::MemCachedPipelineCache()
:	m_accessRead(true)
,	m_accessWrite(true)
{
}

MemCachedPipelineCache::~MemCachedPipelineCache()
{
	destroy();
}

bool MemCachedPipelineCache::create(const PropertyGroup* settings)
{
	std::wstring host = settings->getProperty< std::wstring >(L"Pipeline.MemCached.Host");
	int32_t port = settings->getProperty< int32_t >(L"Pipeline.MemCached.Port", 11211);

	m_socket = new net::TcpSocket();
	if (!m_socket->connect(net::SocketAddressIPv4(host, port)))
		return false;

	m_accessRead = settings->getProperty< bool >(L"Pipeline.MemCached.Read", true);
	m_accessWrite = settings->getProperty< bool >(L"Pipeline.MemCached.Write", true);
	m_proto = new MemCachedProto(m_socket);

	return true;
}

void MemCachedPipelineCache::destroy()
{
	safeClose(m_socket);
	m_proto = nullptr;
}

Ref< IStream > MemCachedPipelineCache::get(const Guid& guid, const PipelineDependencyHash& hash)
{
	if (m_accessRead)
	{
		Ref< MemCachedGetStream > stream = new MemCachedGetStream(m_proto, generateKey(guid, hash));

		// Request end block; do not try to open non-finished cache streams.
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
		return new MemCachedPutStream(m_proto, generateKey(guid, hash));
	else
		return nullptr;
}

	}
}
