#ifndef traktor_db_DefaultFileStore_H
#define traktor_db_DefaultFileStore_H

#include "Database/Local/IFileStore.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_LOCAL_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

/*! \brief Default file store.
 * \ingroup Database
 */
class T_DLLCLASS DefaultFileStore : public IFileStore
{
	T_RTTI_CLASS;

public:
	virtual bool create(const ConnectionString& connectionString) T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual bool pending(const Path& filePath) T_OVERRIDE T_FINAL;

	virtual bool add(const Path& filePath) T_OVERRIDE T_FINAL;

	virtual bool remove(const Path& filePath) T_OVERRIDE T_FINAL;

	virtual bool edit(const Path& filePath) T_OVERRIDE T_FINAL;

	virtual bool rollback(const Path& filePath) T_OVERRIDE T_FINAL;

	virtual bool clean(const Path& filePath) T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_db_DefaultFileStore_H
