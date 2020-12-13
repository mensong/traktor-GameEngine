#pragma once

#include "Core/Object.h"
#include "Core/Containers/SmallMap.h"
#include "Render/Vulkan/ApiHeader.h"

namespace traktor
{
	namespace render
	{
	
class PipelineLayoutCache : public Object
{
public:
	explicit PipelineLayoutCache(VkDevice logicalDevice);

	virtual ~PipelineLayoutCache();

	bool get(uint32_t pipelineHash, const VkDescriptorSetLayoutCreateInfo& dlci, VkDescriptorSetLayout& outDescriptorSetLayout, VkPipelineLayout& outPipelineLayout);

private:
	struct Entry
	{
		VkDescriptorSetLayout descriptorSetLayout;
		VkPipelineLayout pipelineLayout;
#if defined(_DEBUG)
		std::wstring debug;
#endif
	};

	VkDevice m_logicalDevice;
	SmallMap< uint32_t, Entry > m_entries;
};

	}
}