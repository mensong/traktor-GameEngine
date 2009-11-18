#ifndef traktor_editor_IPipelineCache_H
#define traktor_editor_IPipelineCache_H

#include "Core/Object.h"
#include "Core/Guid.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IStream;

	namespace editor
	{

class Settings;

class T_DLLCLASS IPipelineCache : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool create(const Settings* settings) = 0;

	virtual void destroy() = 0;

	virtual Ref< IStream > get(const Guid& guid, uint32_t hash) = 0;

	virtual Ref< IStream > put(const Guid& guid, uint32_t hash) = 0;
};

	}
}

#endif	// traktor_editor_IPipelineCache_H
