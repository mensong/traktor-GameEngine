#ifndef traktor_amalgam_GameEntityData_H
#define traktor_amalgam_GameEntityData_H

#include "Core/RefArray.h"
#include "World/EntityData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_GAME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class IEntityComponentData;

	}

	namespace amalgam
	{

/*! \brief
 * \ingroup Amalgam
 */
class T_DLLCLASS GameEntityData : public world::EntityData
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s);

private:
	friend class GameEntityFactory;
	friend class GameEntityPipeline;

	RefArray< world::IEntityComponentData > m_components;
	Ref< world::EntityData > m_entityData;
};

	}
}

#endif	// traktor_amalgam_GameEntityData_H
