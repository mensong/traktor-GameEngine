#ifndef traktor_sound_StaticSoundResource_H
#define traktor_sound_StaticSoundResource_H

#include "Core/RefArray.h"
#include "Sound/ISoundResource.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

class BankSound;
class IGrain;

/*! \brief Bank sound resource.
 * \ingroup Sound
 */
class T_DLLCLASS BankResource : public ISoundResource
{
	T_RTTI_CLASS;

public:
	BankResource();

	BankResource(
		const RefArray< IGrain >& grains,
		const RefArray< BankSound >& sounds
	);

	virtual Ref< Sound > createSound(resource::IResourceManager* resourceManager, db::Instance* resourceInstance) const;

	virtual bool serialize(ISerializer& s);

	const RefArray< IGrain >& getGrains() const { return m_grains; }

	const RefArray< BankSound >& getSounds() const { return m_sounds; }

private:
	RefArray< IGrain > m_grains;
	RefArray< BankSound > m_sounds;
};

	}
}

#endif	// traktor_sound_StaticSoundResource_H
