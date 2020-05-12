#include "Core/Containers/StaticSet.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderTargetSet.h"
#include "Render/Context/RenderBlock.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/Frame/RenderGraphTargetSetPool.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderGraph", RenderGraph, Object)

RenderGraph::RenderGraph(IRenderSystem* renderSystem)
:	m_nextTargetSetId(1)
{
	m_pool = new RenderGraphTargetSetPool(renderSystem);
}

void RenderGraph::destroy()
{
	m_targets.clear();
	m_passes.clear();
	m_order.clear();
	m_pool = nullptr;
}

handle_t RenderGraph::addTransientTargetSet(
	const wchar_t* const name,
	const RenderGraphTargetSetDesc& targetSetDesc,
	IRenderTargetSet* sharedDepthStencilTargetSet,
	handle_t sizeReferenceTargetSetId
)
{
	handle_t targetSetId = m_nextTargetSetId++;

	auto& target = m_targets[targetSetId];
	target.name = name;
	target.targetSetDesc = targetSetDesc;
	target.sharedDepthStencilTargetSet = sharedDepthStencilTargetSet;
	target.sizeReferenceTargetSetId = sizeReferenceTargetSetId;
	target.referenceCount = 0;
	target.storeDepth = false;
	target.transient = true;

	if (sharedDepthStencilTargetSet != nullptr || targetSetDesc.createDepthStencil || targetSetDesc.usingPrimaryDepthStencil)
		target.storeDepth = true;

	return targetSetId;
}

handle_t RenderGraph::addPersistentTargetSet(
	const wchar_t* const name,
	const RenderGraphTargetSetDesc& targetSetDesc,
	IRenderTargetSet* sharedDepthStencilTargetSet,
	handle_t sizeReferenceTargetSetId
)
{
	handle_t targetSetId = m_nextTargetSetId++;

	auto& target = m_targets[targetSetId];
	target.name = name;
	target.targetSetDesc = targetSetDesc;
	target.sharedDepthStencilTargetSet = sharedDepthStencilTargetSet;
	target.sizeReferenceTargetSetId = sizeReferenceTargetSetId;
	target.referenceCount = 0;
	target.storeDepth = false;
	target.transient = false;

	if (sharedDepthStencilTargetSet != nullptr || targetSetDesc.createDepthStencil || targetSetDesc.usingPrimaryDepthStencil)
		target.storeDepth = true;

	return targetSetId;
}

handle_t RenderGraph::addExternalTargetSet(const wchar_t* const name, IRenderTargetSet* targetSet)
{
	T_FATAL_ASSERT(targetSet != nullptr);
	T_ASSERT(targetSet->getWidth() > 0);
	T_ASSERT(targetSet->getHeight() > 0);

	handle_t targetSetId = m_nextTargetSetId++;

	auto& target = m_targets[targetSetId];
	target.name = name;
	target.rts = targetSet;
	target.sizeReferenceTargetSetId = 0;
	target.referenceCount = 0;
	target.storeDepth = true;
	target.transient = false;

	return targetSetId;
}

handle_t RenderGraph::findTargetByName(const wchar_t* const name) const
{
	for (const auto& tm : m_targets)
	{
		if (wcscmp(tm.second.name, name) == 0)
			return tm.first;
	}
	return 0;
}

IRenderTargetSet* RenderGraph::getTargetSet(handle_t targetSetId) const
{
	auto it = m_targets.find(targetSetId);
	if (it != m_targets.end())
		return it->second.rts;
	else
		return nullptr;
}

void RenderGraph::addPass(const RenderPass* pass)
{
	m_passes.push_back(pass);
}

bool RenderGraph::validate()
{
	StaticSet< uint32_t, 512 > added;

	// Collect order of passes, depth-first.
	m_order.resize(0);	
	for (int32_t i = 0; i < (int32_t)m_passes.size(); ++i)
	{
		if (m_passes[i]->haveOutput())
		{
			const auto& output = m_passes[i]->getOutput();
			if (output.targetSetId == 0)
			{
				// "Render to primary" passes is included as roots.
				traverse(i, [&](int32_t index) {
					if (added.insert(index))
						m_order.push_back(index);
				});
			}
			else
			{
				// Non-transient outputs indicate some sort of caching scheme, such
				// as offscreen caching etc, so we need to ensure they are executed
				// regardless of dependencies.
				auto it = m_targets.find(output.targetSetId);
				if (!it->second.transient)
				{
					traverse(i, [&](int32_t index) {
						if (added.insert(index))
							m_order.push_back(index);
					});
				}
			}
		}
		else
		{
			// Passes which doesn't have an output need to be included as roots.
			traverse(i, [&](int32_t index) {
				if (added.insert(index))
					m_order.push_back(index);
			});
		}
	}

	// Calculate target reference counts,
	// and set storage atttribute of each pass's output.
	for (auto index : m_order)
	{
		const auto pass = m_passes[index];
		auto inputs = pass->getInputs();
		for (const auto& input : inputs)
		{
			if (input.targetSetId == 0)
				continue;

			auto it = m_targets.find(input.targetSetId);
			if (it == m_targets.end())
				return false;

			auto& target = it->second;
			target.referenceCount++;
			target.storeDepth |= input.useDepth;
		}
	}

	return true;
}

bool RenderGraph::build(RenderContext* renderContext, int32_t width, int32_t height)
{
	T_FATAL_ASSERT(!renderContext->havePendingDraws());

	// Render passes in dependency order.
	for (auto index : m_order)
	{
		const auto pass = m_passes[index];
		const auto inputs = pass->getInputs();
		const auto& output = pass->getOutput();

		// Begin render pass.
		if (pass->haveOutput())
		{
			if (output.targetSetId != 0)
			{
				auto it = m_targets.find(output.targetSetId);
				T_FATAL_ASSERT(it != m_targets.end());

				auto& target = it->second;
				if (target.rts == nullptr)
				{
					int32_t referenceWidth = width;
					int32_t referenceHeight = height;

					// Use size of reference target.
					if (target.sizeReferenceTargetSetId != 0)
					{
						auto it2 = m_targets.find(target.sizeReferenceTargetSetId);
						if (it2 == m_targets.end())
							return false;

						referenceWidth = it2->second.targetSetDesc.width;
						referenceHeight = it2->second.targetSetDesc.height;
					}

					// Use size of shared depth/stencil target since they must match.
					if (target.sharedDepthStencilTargetSet != nullptr)
					{
						referenceWidth = target.sharedDepthStencilTargetSet->getWidth();
						referenceHeight = target.sharedDepthStencilTargetSet->getHeight();
					}

					target.rts = m_pool->acquire(
						target.targetSetDesc,
						target.sharedDepthStencilTargetSet,
						referenceWidth,
						referenceHeight
					);
					if (!target.rts)
						return false;
				}	

				auto tb = renderContext->alloc< BeginPassRenderBlock >(pass->getName());
				tb->renderTargetSet = target.rts;
				tb->clear = output.clear;
				tb->load = TfColor | TfDepth;
				tb->store = TfColor | (target.storeDepth ? TfDepth : 0);
				renderContext->enqueue(tb);
			}
			else
			{
				auto tb = renderContext->alloc< BeginPassRenderBlock >(pass->getName());
				tb->clear = output.clear;
				tb->load = TfColor | TfDepth;
				tb->store = TfColor | TfDepth;
				renderContext->enqueue(tb);			
			}
		}

		// Build sub passes first.
		for (auto subPass : pass->getSubPasses())
		{
			for (const auto& build : subPass->getBuilds())
			{
				build(*this, renderContext);
				T_FATAL_ASSERT(!renderContext->havePendingDraws());
			}
		}

		// Build this pass.
		for (const auto& build : pass->getBuilds())
		{
			build(*this, renderContext);
			T_FATAL_ASSERT(!renderContext->havePendingDraws());
		}

		// End render pass.
		if (pass->haveOutput())
		{
			auto te = renderContext->alloc< EndPassRenderBlock >();
			renderContext->enqueue(te);
		}

		// Decrement reference counts on input targets; release if last reference.
		for (const auto& input : inputs)
		{
			if (input.targetSetId == 0)
				continue;

			auto it = m_targets.find(input.targetSetId);
			if (it == m_targets.end())
				return false;

			auto& target = it->second;
			if (--target.referenceCount <= 0)
			{
				if (target.transient)
					m_pool->release(target.rts);
				target.rts = nullptr;
			}
		}
	}

	T_FATAL_ASSERT(!renderContext->havePendingDraws());

	// Cleanup pool data structure.
	m_pool->cleanup();

	// Remove all data; keep memory allocated for arrays
	// since it's very likely this will be identically
	// re-populated next frame.
	m_targets.reset();
	m_passes.resize(0);
	m_order.resize(0);
	return true;
}

const SmallMap< handle_t, RenderGraph::Target >& RenderGraph::getTargets() const
{
	return m_targets;
}

void RenderGraph::traverse(int32_t index, const std::function< void(int32_t) >& fn) const
{
	for (const auto& input : m_passes[index]->getInputs())
	{
		for (int32_t i = 0; i < m_passes.size(); ++i)
		{
			if (i == index)
				continue;
			if (m_passes[i]->getOutput().targetSetId == input.targetSetId)
				traverse(i, fn);
		}
	}
	fn(index);
}

	}
}