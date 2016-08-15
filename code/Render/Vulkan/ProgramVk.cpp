#if defined(_WIN32)
#	include "Render/Vulkan/Win32/ApiLoader.h"
#endif
#include "Render/Vulkan/ContextVk.h"
#include "Render/Vulkan/ProgramResourceVk.h"
#include "Render/Vulkan/ProgramVk.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ProgramVk", ProgramVk, IProgram)

ProgramVk::ProgramVk(ContextVk* sharedContext)
:	m_sharedContext(sharedContext)
{
}

ProgramVk::~ProgramVk()
{
	destroy();
}

bool ProgramVk::create(const ProgramResourceVk* resource)
{
	// Create vertex shader module.
	VkShaderModuleCreateInfo vertexShaderCreationInfo = {};
	vertexShaderCreationInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	vertexShaderCreationInfo.codeSize = resource->m_vertexShader.size() * sizeof(uint32_t);
	vertexShaderCreationInfo.pCode = &resource->m_vertexShader[0];
 
	VkShaderModule vertexShaderModule;
	if (vkCreateShaderModule(m_sharedContext->getVkDevice(), &vertexShaderCreationInfo, NULL, &vertexShaderModule) != VK_SUCCESS)
		return false;

	// Create fragment shader module.

	return true;
}

void ProgramVk::destroy()
{
}

void ProgramVk::setFloatParameter(handle_t handle, float param)
{
	setFloatArrayParameter(handle, &param, 1);
}

void ProgramVk::setFloatArrayParameter(handle_t handle, const float* param, int length)
{
}

void ProgramVk::setVectorParameter(handle_t handle, const Vector4& param)
{
	setVectorArrayParameter(handle, &param, 1);
}

void ProgramVk::setVectorArrayParameter(handle_t handle, const Vector4* param, int length)
{
}

void ProgramVk::setMatrixParameter(handle_t handle, const Matrix44& param)
{
}

void ProgramVk::setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length)
{
}

void ProgramVk::setTextureParameter(handle_t handle, ITexture* texture)
{
}

void ProgramVk::setStencilReference(uint32_t stencilReference)
{
}

	}
}
