#ifndef traktor_sound_SoundFactory_H
#define traktor_sound_SoundFactory_H

#include "Core/Heap/Ref.h"
#include "Resource/ResourceFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

class Database;

	}

	namespace sound
	{

class SoundSystem;

/*! \brief Sound resource factory.
 * \ingroup Sound
 */
class T_DLLCLASS SoundFactory : public resource::ResourceFactory
{
	T_RTTI_CLASS(SoundFactory)

public:
	SoundFactory(db::Database* db, SoundSystem* soundSystem);

	virtual const TypeSet getResourceTypes() const;

	virtual Object* create(const Type& resourceType, const Guid& guid);

private:
	Ref< db::Database > m_db;
	Ref< SoundSystem > m_soundSystem;
};

	}
}

#endif	// traktor_sound_SoundFactory_H
