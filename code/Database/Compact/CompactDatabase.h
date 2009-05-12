#ifndef traktor_db_CompactDatabase_H
#define traktor_db_CompactDatabase_H

#include "Core/Heap/Ref.h"
#include "Database/Provider.h"
#include "Core/Misc/MD5.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Path;

	namespace db
	{

class CompactContext;
class CompactGroup;

/*! \brief Compact database provider
 * \ingroup Database
 */
class T_DLLCLASS CompactDatabase : public IProviderDatabase
{
	T_RTTI_CLASS(CompactDatabase)

public:
	bool create(const Path& filePath);

	bool open(const Path& filePath, bool readOnly = false);

	virtual void close();

	virtual IProviderBus* getBus();

	virtual IProviderGroup* getRootGroup();

private:
	Ref< CompactContext > m_context;
	Ref< CompactGroup > m_rootGroup;
	bool m_readOnly;
	MD5 m_registryHash;
};

	}
}

#endif	// traktor_db_CompactDatabase_H
