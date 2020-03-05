#pragma once

#include "Render/Vulkan/VertexBufferVk.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup Vulkan
 */
class VertexBufferDynamicVk : public VertexBufferVk
{
	T_RTTI_CLASS;

public:
	VertexBufferDynamicVk(
		uint32_t bufferSize,
		VmaAllocator allocator,
		VmaAllocation allocation,
		VkBuffer vertexBuffer,
		const VkVertexInputBindingDescription& vertexBindingDescription,
		const AlignedVector< VkVertexInputAttributeDescription >& vertexAttributeDescriptions,
		uint32_t hash
	);

	virtual void* lock() override final;

	virtual void* lock(uint32_t vertexOffset, uint32_t vertexCount) override final;

	virtual void unlock() override final;

private:
	bool m_locked;
};

	}
}

