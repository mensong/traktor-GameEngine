#ifndef traktor_video_VideoFactory_H
#define traktor_video_VideoFactory_H

#include "Resource/IResourceFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_VIDEO_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

class Database;

	}

	namespace render
	{

class IRenderSystem;

	}

	namespace video
	{

/*! \brief Video resource factory.
 * \ingroup Video
 */
class T_DLLCLASS VideoFactory : public resource::IResourceFactory
{
	T_RTTI_CLASS;

public:
	VideoFactory(db::Database* database, render::IRenderSystem* renderSystem);

	virtual const TypeInfoSet getResourceTypes() const T_OVERRIDE T_FINAL;

	virtual const TypeInfoSet getProductTypes() const T_OVERRIDE T_FINAL;

	virtual bool isCacheable() const T_OVERRIDE T_FINAL;

	virtual Ref< Object > create(resource::IResourceManager* resourceManager, const TypeInfo& resourceType, const Guid& guid, const Object* current) const T_OVERRIDE T_FINAL;

private:
	Ref< db::Database > m_database;
	Ref< render::IRenderSystem > m_renderSystem;
};

	}
}

#endif	// traktor_video_VideoFactory_H
