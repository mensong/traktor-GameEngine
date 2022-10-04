#pragma once

#include <functional>
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Thread/Semaphore.h"
#include "Render/Vulkan/Private/ApiHeader.h"

namespace traktor::render
{

class Queue;
class UniformBufferPool;

/*! Render system shared context.
 *
 * This context is owned by render system and shared
 * across all render views.
 */
class Context : public Object
{
	T_RTTI_CLASS;

public:
	typedef std::function< void(Context*) > cleanup_fn_t;

	explicit Context(
		VkPhysicalDevice physicalDevice,
		VkDevice logicalDevice,
		VmaAllocator allocator,
		uint32_t graphicsQueueIndex
	);

	virtual ~Context();

	void incrementViews();

	void decrementViews();

	void addDeferredCleanup(const cleanup_fn_t& fn);

	bool needCleanup() const;

	void performCleanup();

	void recycle();

	bool savePipelineCache();

	VkPhysicalDevice getPhysicalDevice() const { return m_physicalDevice; }

	VkDevice getLogicalDevice() const { return m_logicalDevice; }

	VmaAllocator getAllocator() const { return m_allocator; }

	VkPipelineCache getPipelineCache() const { return m_pipelineCache; }

	VkDescriptorPool getDescriptorPool() const { return m_descriptorPool; }

	uint32_t getDescriptorPoolRevision() const { return m_descriptorPoolRevision; }

	Queue* getGraphicsQueue() const { return m_graphicsQueue; }

	UniformBufferPool* getUniformBufferPool(int32_t index) const { return m_uniformBufferPools[index]; }

private:
	VkPhysicalDevice m_physicalDevice;
	VkDevice m_logicalDevice;
	VmaAllocator m_allocator;
	VkPipelineCache m_pipelineCache;
	VkDescriptorPool m_descriptorPool;
	int32_t m_views;
	uint32_t m_descriptorPoolRevision;
	Ref< Queue > m_graphicsQueue;
	Ref< UniformBufferPool > m_uniformBufferPools[3];
	Semaphore m_cleanupLock;
	AlignedVector< cleanup_fn_t > m_cleanupFns;
};

}
