#ifndef traktor_editor_MemCachedPipelineCache_H
#define traktor_editor_MemCachedPipelineCache_H

#include "Editor/IPipelineCache.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace net
	{

class TcpSocket;

	}

	namespace editor
	{

class MemCachedProto;

class T_DLLCLASS MemCachedPipelineCache : public IPipelineCache
{
	T_RTTI_CLASS;

public:
	MemCachedPipelineCache();

	virtual ~MemCachedPipelineCache();

	virtual bool create(const Settings* settings);

	virtual void destroy();

	virtual Ref< IStream > get(const Guid& guid, uint32_t hash);

	virtual Ref< IStream > put(const Guid& guid, uint32_t hash);

private:
	Ref< net::TcpSocket > m_socket;
	Ref< MemCachedProto > m_proto;
	bool m_accessRead;
	bool m_accessWrite;
};

	}
}

#endif	// traktor_editor_MemCachedPipelineCache_H
