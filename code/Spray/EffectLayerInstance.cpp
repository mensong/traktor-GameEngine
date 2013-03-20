#include "Spray/EffectLayerInstance.h"
#include "Spray/EffectLayer.h"
#include "Spray/EmitterInstance.h"
#include "Spray/ITriggerInstance.h"
#include "Spray/SequenceInstance.h"

namespace traktor
{
	namespace spray
	{
		namespace
		{

const float c_singleShotThreshold = 0.01f;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.EffectLayerInstance", EffectLayerInstance, Object)

EffectLayerInstance::EffectLayerInstance(
	const EffectLayer* layer,
	EmitterInstance* emitterInstance,
	SequenceInstance* sequenceInstance,
	ITriggerInstance* triggerInstanceEnable,
	ITriggerInstance* triggerInstanceDisable
)
:	m_layer(layer)
,	m_emitterInstance(emitterInstance)
,	m_sequenceInstance(sequenceInstance)
,	m_triggerInstanceEnable(triggerInstanceEnable)
,	m_triggerInstanceDisable(triggerInstanceDisable)
,	m_start(m_layer->getTime())
,	m_end(m_layer->getTime() + m_layer->getDuration())
,	m_singleShotFired(false)
,	m_enable(false)
{
}

void EffectLayerInstance::update(Context& context, const Transform& transform, float time, bool enable)
{
	if (m_emitterInstance)
	{
		if (m_end > m_start + c_singleShotThreshold)
		{
			if (time >= m_start && time <= m_end)
				m_emitterInstance->update(context, transform, enable, false);
			else if (time > m_end)
				m_emitterInstance->update(context, transform, false, false);
		}
		else
		{
			if (time >= m_start && !m_singleShotFired)
			{
				m_emitterInstance->update(context, transform, enable, true);
				m_singleShotFired = true;
			}
			else if (time > m_end)
				m_emitterInstance->update(context, transform, false, false);
		}
	}

	if (m_sequenceInstance)
	{
		if (time >= m_start - FUZZY_EPSILON)
			m_sequenceInstance->update(context, transform, time - m_start, enable);
	}

	if (m_triggerInstanceEnable || m_triggerInstanceDisable)
	{
		if (enable != m_enable)
		{
			if (m_triggerInstanceEnable)
				m_triggerInstanceEnable->perform(context, transform, enable);
			if (m_triggerInstanceDisable)
				m_triggerInstanceDisable->perform(context, transform, !enable);

			m_enable = enable;
		}

		if (m_triggerInstanceEnable)
			m_triggerInstanceEnable->update(context, transform, m_enable);
		if (m_triggerInstanceDisable)
			m_triggerInstanceDisable->update(context, transform, !m_enable);
	}
}

void EffectLayerInstance::synchronize()
{
	if (m_emitterInstance)
		m_emitterInstance->synchronize();
}

void EffectLayerInstance::render(PointRenderer* pointRenderer, const Transform& transform, const Plane& cameraPlane, float time) const
{
	if (m_emitterInstance && time >= m_start)
		m_emitterInstance->render(pointRenderer, transform, cameraPlane);
}

Aabb3 EffectLayerInstance::getBoundingBox() const
{
	return m_emitterInstance ? m_emitterInstance->getBoundingBox() : Aabb3();
}

	}
}
