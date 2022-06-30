#include "Spray/Effect.h"
#include "Spray/EffectInstance.h"
#include "Spray/EffectLayer.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.Effect", Effect, Object)

Effect::Effect(
	float duration,
	float loopStart,
	float loopEnd,
	const RefArray< EffectLayer >& layers
)
:	m_duration(duration)
,	m_loopStart(loopStart)
,	m_loopEnd(loopEnd)
,	m_layers(layers)
{
}

Ref< EffectInstance > Effect::createInstance() const
{
	Ref< EffectInstance > effectInstance = new EffectInstance(this);
	for (auto layer : m_layers)
	{
		Ref< EffectLayerInstance > layerInstance = layer->createInstance();
		if (layerInstance)
			effectInstance->m_layerInstances.push_back(layerInstance);
	}
	return effectInstance;
}

	}
}
