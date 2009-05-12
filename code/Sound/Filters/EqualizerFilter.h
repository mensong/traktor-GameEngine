#ifndef traktor_sound_EqualizerFilter_H
#define traktor_sound_EqualizerFilter_H

#include "Sound/Filter.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

/*! \brief Equalizer filter.
 * \ingroup Sound
 */
class T_DLLCLASS EqualizerFilter : public Filter
{
	T_RTTI_CLASS(EqualizerFilter)

public:
	EqualizerFilter(float gain);

	virtual void apply(SoundBlock& outBlock);

private:
	float m_gain;
	float m_historySamples[2][2];
	float m_historyFiltered[2][2];
};

	}
}

#endif	// traktor_sound_EqualizerFilter_H
