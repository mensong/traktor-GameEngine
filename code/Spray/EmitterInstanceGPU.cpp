/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Buffer.h"
#include "Render/IRenderSystem.h"
#include "Render/Context/RenderBlock.h"
#include "Render/Context/RenderContext.h"
#include "Resource/IResourceManager.h"
#include "Spray/Emitter.h"
#include "Spray/EmitterInstanceGPU.h"
#include "Spray/PointRenderer.h"
#include "Spray/Source.h"
#include "Spray/Vertex.h"

namespace traktor::spray
{
	namespace
	{
	
const resource::Id< render::Shader > c_shaderLifetime(L"{A83B0679-4DA7-7B4C-92F4-7A17738B8804}");
const resource::Id< render::Shader > c_shaderEvolve(L"{BAA7E3FC-7E27-4FD9-96D8-DC8CDD084E4C}");

const uint32_t c_maxPointCount = 10000;

const render::Handle s_handleSeed(L"Spray_Seed");
const render::Handle s_handleDeltaTime(L"Spray_DeltaTime");
const render::Handle s_handleEmitCount(L"Spray_EmitCount");
const render::Handle s_handleTransform(L"Spray_Transform");
const render::Handle s_handleHead(L"Spray_Head");
const render::Handle s_handlePoints(L"Spray_Points");

// Buffer containing information about particle system instance.
// Suitable for indirect draw thus IndexedIndirectDraw must be at top.
struct Head
{
	render::IndexedIndirectDraw indirectDraw;
	int32_t capacity;
	int32_t alive;
};

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.EmitterInstanceGPU", EmitterInstanceGPU, IEmitterInstance)

Ref< EmitterInstanceGPU > EmitterInstanceGPU::createInstance(render::IRenderSystem* renderSystem, resource::IResourceManager* resourceManager, const Emitter* emitter, float duration)
{
	Ref< EmitterInstanceGPU > emitterInstance = new EmitterInstanceGPU(emitter);

	// Bind shaders.
	if (!resourceManager->bind(c_shaderLifetime, emitterInstance->m_shaderLifetime))
		return nullptr;
	if (!resourceManager->bind(c_shaderEvolve, emitterInstance->m_shaderEvolve))
		return nullptr;

	if (emitter->getSource() != nullptr && emitter->getSource()->getShader().isValid())
	{
		if (!resourceManager->bind(emitter->getSource()->getShader(), emitterInstance->m_shaderSource))
			return nullptr;
	}

	// Create buffers.
	emitterInstance->m_headBuffer = renderSystem->createBuffer(render::BuStructured | render::BuIndirect, sizeof(Head), false);
	if (!emitterInstance->m_headBuffer)
		return nullptr;

	emitterInstance->m_pointBuffer = renderSystem->createBuffer(render::BuStructured, c_maxPointCount * sizeof(Point), false);
	if (!emitterInstance->m_pointBuffer)
		return nullptr;

	// Prepare head data.
	Head* head = (Head*)emitterInstance->m_headBuffer->lock();
	head->indirectDraw.indexCount = 6;
	head->indirectDraw.instanceCount = 0;
	head->indirectDraw.firstIndex = 0;
	head->indirectDraw.vertexOffset = 0;
	head->indirectDraw.firstInstance = 0;
	head->capacity = c_maxPointCount;
	head->alive = 0;
	emitterInstance->m_headBuffer->unlock();

	return emitterInstance;
}

EmitterInstanceGPU::~EmitterInstanceGPU()
{
	synchronize();
}

void EmitterInstanceGPU::update(Context& context, const Transform& transform, bool emit, bool singleShot)
{
	const Vector4 lastPosition = m_transform.translation();
	m_transform = transform;
	m_pendingSeed = context.random.next();

	if (emit)
	{
		const Source* source = m_emitter->getSource();
		if (source)
		{
			if (!singleShot)
			{
				const Vector4 dm = (lastPosition - m_transform.translation()).xyz0();

				const float emitVelocity = context.deltaTime > FUZZY_EPSILON ? source->getVelocityRate() * (dm.length() / context.deltaTime) : 0.0f;
				const float emitConstant = source->getConstantRate() * context.deltaTime + m_emitFraction;
				const float emitTotal = emitVelocity + emitConstant;

				m_pendingEmit = (int32_t)emitTotal;
				m_emitFraction = emitTotal - m_pendingEmit;
			}
			else
			{
				m_pendingEmit = (int32_t)source->getConstantRate();
				m_emitFraction = 0.0f;
			}
		}
	}
	else
	{
		m_pendingEmit = 0;
		m_emitFraction = 0.0f;
	}

	m_pendingDeltaTime += context.deltaTime;
	m_needLifetimeUpdate = true;
}

void EmitterInstanceGPU::render(
	render::handle_t technique,
	render::RenderContext* renderContext,
	PointRenderer* pointRenderer,
	MeshRenderer* meshRenderer,
	TrailRenderer* trailRenderer,
	const Transform& transform,
	const Vector4& cameraPosition,
	const Plane& cameraPlane
)
{
	const float distance = 0.0f;

	if (!m_emitter->getShader()->hasTechnique(technique))
		return;

	if (m_needLifetimeUpdate)
	{
		// Update life time of points; prepare indirect draw.
		{
			auto rb = renderContext->alloc< render::ComputeRenderBlock >();
			rb->program = m_shaderLifetime->getProgram().program;
			rb->programParams = renderContext->alloc< render::ProgramParameters >();
			rb->programParams->beginParameters(renderContext);
			rb->programParams->setBufferViewParameter(s_handleHead, m_headBuffer->getBufferView());
			rb->programParams->setBufferViewParameter(s_handlePoints, m_pointBuffer->getBufferView());
			rb->programParams->endParameters(renderContext);
			renderContext->compute(rb);
		}

		renderContext->compute< render::BarrierRenderBlock >(render::Stage::Compute, render::Stage::Compute, nullptr, 0);

		// Evolve points; prepare indirect draw.
		{
			auto rb = renderContext->alloc< render::ComputeRenderBlock >();
			rb->program = m_shaderEvolve->getProgram().program;
			rb->programParams = renderContext->alloc< render::ProgramParameters >();
			rb->programParams->beginParameters(renderContext);
			rb->programParams->setFloatParameter(s_handleDeltaTime, m_pendingDeltaTime);
			rb->programParams->setBufferViewParameter(s_handleHead, m_headBuffer->getBufferView());
			rb->programParams->setBufferViewParameter(s_handlePoints, m_pointBuffer->getBufferView());
			rb->programParams->endParameters(renderContext);
			renderContext->compute(rb);			
		}

		// Emit new points.
		if (m_emitter->getSource() && m_shaderSource && m_pendingEmit > 0)
		{
			renderContext->compute< render::BarrierRenderBlock >(render::Stage::Compute, render::Stage::Compute, nullptr, 0);

			auto rb = renderContext->alloc< render::ComputeRenderBlock >();
			rb->program = m_shaderSource->getProgram().program;
			rb->programParams = renderContext->alloc< render::ProgramParameters >();
			rb->programParams->beginParameters(renderContext);
			rb->programParams->setFloatParameter(s_handleSeed, (float)m_pendingSeed);
			rb->programParams->setFloatParameter(s_handleDeltaTime, m_pendingDeltaTime);
			rb->programParams->setFloatParameter(s_handleEmitCount, (float)m_pendingEmit);
			rb->programParams->setMatrixParameter(s_handleTransform, m_transform.toMatrix44());
			rb->programParams->setBufferViewParameter(s_handleHead, m_headBuffer->getBufferView());
			rb->programParams->setBufferViewParameter(s_handlePoints, m_pointBuffer->getBufferView());
			m_emitter->getSource()->setShaderParameters(rb->programParams);
			rb->programParams->endParameters(renderContext);
			renderContext->compute(rb);
		}

		renderContext->compute< render::BarrierRenderBlock >(render::Stage::Compute, render::Stage::Vertex | render::Stage::Indirect, nullptr, 0);

		m_pendingDeltaTime = 0.0f;
		m_needLifetimeUpdate = false;
	}

	pointRenderer->batchUntilFlush(
		m_emitter->getShader(),
		m_headBuffer,
		m_pointBuffer,
		distance
	);
}

void EmitterInstanceGPU::synchronize() const
{
}

EmitterInstanceGPU::EmitterInstanceGPU(const Emitter* emitter)
:	m_emitter(emitter)
,	m_needLifetimeUpdate(false)
{
}

}
