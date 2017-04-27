/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/Vulkan/ContextVk.h"

namespace traktor
{
	namespace render
	{

ContextVk::ContextVk(VkInstance vkInstance, VkSurfaceKHR vkSurface, VkPhysicalDevice vkPhysicalDevice, VkDevice vkDevice, VkQueue vkPresentQueue)
:	m_vkInstance(vkInstance)
,	m_vkSurface(vkSurface)
,	m_vkPhysicalDevice(vkPhysicalDevice)
,	m_vkDevice(vkDevice)
,	m_vkPresentQueue(vkPresentQueue)
{
}

	}
}